#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>

#include "Common.h"
#include "ParticleSpawner.h"
#include "ParticleRenderer.h"
//#include "DEsolver/SimpleDEsolver.h"
#include "DEsolver/Leapfrog.h"
//#include "DEsolver/Verlet.h"
//#include "DEsolver/VelocityVerlet.h"
//#include "DEsolver/RungeKutta4.h"

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
    mpu::gph::enableVsync(false);

    // generate some particles
    ParticleSpawner spawner;
    spawner.spawnParticles(NUM_PARTICLES,TOTAL_MASS,TEMPERATURE, 2);
    auto pb = spawner.getParticleBuffer();

    // create a renderer
    ParticleRenderer renderer;
    renderer.setParticleBuffer(pb);
    renderer.setShaderSettings(Falloff::LINEAR);
    renderer.enableAdditiveBlending(true);
    renderer.enableDepthTest(false);
    renderer.setViewportSize({WIDTH,HEIGHT});
    renderer.setColor({0.9,0.3,0.1,1});
    renderer.setBrightness(1);
    renderer.setSize(PARTICLE_RENDER_SIZE);

    // create camera
    mpu::gph::Camera camera(std::make_shared<mpu::gph::SimpleWASDController>(&window,10,4));
    camera.setMVP(&renderer);
    camera.setClip(0.1,100);

    // create shaders for acceleration
    uint32_t accWgSize = calcWorkgroupSize(NUM_PARTICLES);
    mpu::gph::ShaderProgram accShader({{PROJECT_SHADER_PATH"Acceleration/nvidia-gravity.comp"}},
                                      {{"WGSIZE",{mpu::toString(accWgSize)}},
                                       {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}}});
    accShader.uniform1f("smoothing_epsilon_squared",  EPS2);
    accShader.uniform1f("gravity_constant",  G);

    uint32_t wgSize = calcWorkgroupSize(NUM_PARTICLES);
    auto accFunc = [accShader,wgSize](){
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        accShader.dispatch(NUM_PARTICLES/wgSize);
//        accShader.dispatch(NUM_PARTICLES,wgSize);
    };

    //  create a simulator
    Leapfrog simulation(accFunc,pb,DT);
    simulation.start();

    float brightness=1;

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;


    // TODO: add gpu stopwatch
    // TODO: performance structure of arrays
    // TODO: put mass into position
    // TODO: performance: work group size calculator
    // TODO: performance test one shader two buffers vs two shaders
    // TODO: 2D mode
    // TODO: fix spawner
    // TODO: other fun  spawning scenarios


    bool runSim = false;
    while( window.update())
    {
        dt = timer.getDeltaTime();
        camera.update(dt);

        if(window.getKey(GLFW_KEY_1) == GLFW_PRESS && lag < 0.4)
            lag += dt;
        else
            lag = 0;

        if(window.getKey(GLFW_KEY_2) != GLFW_PRESS)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // make brightness adjustable
        if(window.getKey(GLFW_KEY_KP_ADD) == GLFW_PRESS)
            brightness += brightness *0.5*dt;
        if(window.getKey(GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
            brightness -= brightness *0.5*dt;
        renderer.setBrightness(brightness);

        while(lag >= DT)
        {
            simulation.advanceTime();
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