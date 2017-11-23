#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>

#include "Common.h"
#include "ParticleSpawner.h"

constexpr int HEIGHT = 800;
constexpr int WIDTH = 800;




int main()
{
    // initialise log
    mpu::Log mainLog(mpu::ALL, mpu::ConsoleSink());

    // create window and init gl
    mpu::gph::Window window(WIDTH,HEIGHT,"GravitySim");

    // create a mvp in gpu memory
    mpu::gph::Buffer ubo;
    ubo.allocate<mpu::gph::ModelViewProjection>(1,GL_MAP_READ_BIT|GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT);
    ubo.bindBase( 0, GL_UNIFORM_BUFFER);
    auto mvp = ubo.map<mpu::gph::ModelViewProjection>(1,0,GL_MAP_READ_BIT|GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT);
    mvp[0] = mpu::gph::ModelViewProjection();

    // create camera
    mpu::gph::Camera camera(&window);
    camera.setMVP(&mvp[0]);

    // set gl options
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);
    glPointSize(2.0f);
    mpu::gph::enableVsync(true);

    // create a shader for rendering
    mpu::gph::addShaderIncludePath(LIB_SHADER_PATH);
    mpu::gph::addShaderIncludePath(PROJECT_SHADER_PATH);
    mpu::gph::ShaderProgram renderShader({ {LIB_SHADER_PATH"simple.frag"},
                                     {LIB_SHADER_PATH"simple.vert"} });
    renderShader.uniform4f("color",{1,1,1,1});

    // generate some particles
    ParticleSpawner spawner;
    spawner.spawnParticles(NUM_PARTICLES,TOTAL_MASS,TEMPERATURE, 2);

    // create a shader for simulation
    mpu::gph::ShaderProgram simulationShader({{PROJECT_SHADER_PATH"naive-gravity.comp"}});
    simulationShader.uniform1f("dt",DT);
    simulationShader.uniform1f("smoothingEpsilonSquared",  EPS2);
    simulationShader.uniform1f("gravityConstant",  G);
    simulationShader.uniform1ui("numOfParticles",  NUM_PARTICLES);
    auto pb = spawner.getParticleBuffer();
    pb.bindBase(PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);

    // create a vao
    mpu::gph::VertexArray vao;
    vao.addAttributeBufferArray(0,spawner.getParticleBuffer(),0,sizeof(Particle),4,mpu::gph::offset_of(&Particle::position));

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;


    bool runSim = false;
    while( window.update())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFinish();

        dt = timer.getDeltaTime();
        camera.update(dt);

        if(runSim)
            lag += dt;
        while(lag >= DT)
        {
            simulationShader.dispatch(NUM_PARTICLES,1000,GL_SHADER_STORAGE_BARRIER_BIT);
            lag -= DT;
        }


        // render the particles
        renderShader.use();
        vao.bind();
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        glDrawArrays( GL_POINTS, 0, NUM_PARTICLES);

        if(window.getKey(GLFW_KEY_1) == GLFW_PRESS && dt < 0.5)
        {
            runSim = true;
        } else {
            runSim = false;
        }

        // performance display
        nbframes++;
        elapsedPerT += dt;
        if(elapsedPerT >= 1.0)
        {
            printf("%f ms/frame -- %f fps\n", 1000.0*elapsedPerT/double(nbframes), nbframes/elapsedPerT);
            nbframes = 0;
            elapsedPerT = 0;
        }
    }

    return 0;
}