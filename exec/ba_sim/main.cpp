#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>

#include "Common.h"
#include "ParticleSpawner.h"
#include "ParticleRenderer.h"

constexpr int HEIGHT = 800;
constexpr int WIDTH = 800;

int main()
{
    // initialise log
    mpu::Log mainLog(mpu::ALL, mpu::ConsoleSink());

    // create window and init gl
    mpu::gph::Window window(WIDTH,HEIGHT,"GravitySim");

    // add the shader include pathes
    mpu::gph::addShaderIncludePath(LIB_SHADER_PATH);
    mpu::gph::addShaderIncludePath(PROJECT_SHADER_PATH);

    // set some gl options
    glClearColor(0, 0, 0, 1);
//    glClearDepth(1.f);
//    glEnable(GL_DEPTH_TEST);
    mpu::gph::enableVsync(false);

    // generate some particles
    ParticleSpawner spawner;
    spawner.spawnParticles(NUM_PARTICLES,TOTAL_MASS,TEMPERATURE, 2);
    auto pb = spawner.getParticleBuffer();
    pb.bindBase(PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);

    // create a renderer
    ParticleRenderer renderer;
    renderer.setViewportSize({WIDTH,HEIGHT});
    renderer.configureArrays(mpu::gph::offset_of(&Particle::position), mpu::gph::offset_of(&Particle::renderSize));
    renderer.setParticleBuffer<Particle>( spawner.getParticleBuffer(), spawner.getNumParticles());

    // create camera
    mpu::gph::Camera camera(&window);
    camera.setMVP(&renderer);

    // create shaders for simulation
    mpu::gph::ShaderProgram accShader({{PROJECT_SHADER_PATH"Acceleration/naive-gravity.comp"}});
    accShader.uniform1f("smoothing_epsilon_squared",  EPS2);
    accShader.uniform1f("gravity_constant",  G);
    accShader.uniform1ui("num_of_particles",  NUM_PARTICLES);

//    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"Integration/semi-implicit-euler.comp"}});
//    integShader.uniform1f("dt",DT);
//    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);

//    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"Integration/leapfrog.comp"}});
//    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    integShader.uniform1f("dt",DT);
//    integShader.uniform1f("vel_dt",DT/2.0);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    accShader.dispatch(NUM_PARTICLES,100);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    integShader.dispatch(NUM_PARTICLES,100);
//    integShader.uniform1f("vel_dt",DT);

    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"Integration/verlet.comp"}});
    mpu::gph::ShaderProgram verletFirstShader({{PROJECT_SHADER_PATH"Integration/verletFirstStep.comp"}});
    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
    verletFirstShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
    integShader.uniform1f("dt",DT);
    verletFirstShader.uniform1f("dt",DT);

    mpu::gph::Buffer verletBuffer;
    verletBuffer.allocate<glm::vec4>(NUM_PARTICLES);
    verletBuffer.bindBase( VERLET_LAST_POS_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    accShader.dispatch(NUM_PARTICLES,100);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    verletFirstShader.dispatch(NUM_PARTICLES,100);


    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;


    bool runSim = false;
    while( window.update())
    {

//        glFinish();

        dt = timer.getDeltaTime();
        camera.update(dt);

        if(window.getKey(GLFW_KEY_1) == GLFW_PRESS && dt < 0.5)
            lag += dt;
        else
            lag = 0;

        if(window.getKey(GLFW_KEY_2) != GLFW_PRESS)
            glClear(GL_COLOR_BUFFER_BIT);

        while(lag >= DT)
        {
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            accShader.dispatch(NUM_PARTICLES,500);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            integShader.dispatch(NUM_PARTICLES,500);

            lag -= DT;
        }

        // render the particles
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        renderer.draw();

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