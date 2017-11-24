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
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);
    mpu::gph::enableVsync(true);

    // generate some particles
    ParticleSpawner spawner;
    spawner.spawnParticles(NUM_PARTICLES,TOTAL_MASS,TEMPERATURE, 2);

    // create a renderer
    ParticleRenderer renderer;
    renderer.setViewportSize({WIDTH,HEIGHT});
    renderer.configureArrays(mpu::gph::offset_of(&Particle::position), mpu::gph::offset_of(&Particle::renderSize));
    renderer.setParticleBuffer<Particle>( spawner.getParticleBuffer(), spawner.getNumParticles());

    // create camera
    mpu::gph::Camera camera(&window);
    camera.setMVP(&renderer);

    // create a shader for simulation
    mpu::gph::ShaderProgram simulationShader({{PROJECT_SHADER_PATH"naive-gravity.comp"}});
    simulationShader.uniform1f("dt",DT);
    simulationShader.uniform1f("smoothingEpsilonSquared",  EPS2);
    simulationShader.uniform1f("gravityConstant",  G);
    simulationShader.uniform1ui("numOfParticles",  NUM_PARTICLES);
    auto pb = spawner.getParticleBuffer();
    pb.bindBase(PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);

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
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            simulationShader.dispatch(NUM_PARTICLES,1000);
            lag -= DT;
        }

        // render the particles
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        renderer.draw();

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