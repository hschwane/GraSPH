#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>

#include "Common.h"
#include "ParticleSpawner.h"
#include "ParticleRenderer.h"
#include "DEsolver/SimpleDEsolver.h"
#include "DEsolver/Leapfrog.h"
#include "DEsolver/Verlet.h"
#include "DEsolver/VelocityVerlet.h"

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
    ParticleBuffer pb(NUM_PARTICLES,THREADS_PER_PARTICLE);
    ParticleSpawner spawner;
    spawner.setBuffer(pb);
    spawner.spawnParticles(TOTAL_MASS,TEMPERATURE, 2);

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
//    uint32_t accWgSize = 128;//calcWorkgroupSize(NUM_PARTICLES*THREADS_PER_PARTICLE);
//    mpu::gph::ShaderProgram accShader({{PROJECT_SHADER_PATH"Acceleration/nBodyGravity/nvidia-gravity.comp"}},
//                                      {
//                                       {"WGSIZE",{mpu::toString(accWgSize)}},
//                                       {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
//                                       {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / accWgSize / THREADS_PER_PARTICLE)}}
//                                      });
//    accShader.uniform1f("smoothing_epsilon_squared",  EPS2);
//    accShader.uniform1f("gravity_constant",  G);
//
//    uint32_t accumWgSize = calcWorkgroupSize(NUM_PARTICLES);
//    mpu::gph::ShaderProgram accAccum({{PROJECT_SHADER_PATH"Acceleration/accAccumulator.comp"}},
//                                      {
//                                       {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
//                                       {"ACCELERATIONS_PER_PARTICLE",{mpu::toString(THREADS_PER_PARTICLE)}}
//                                      });
//
//    auto accFunc = [accShader,accWgSize,accAccum,accumWgSize](){
//        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//        accShader.dispatch(NUM_PARTICLES*THREADS_PER_PARTICLE/accWgSize);
//        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//        accAccum.dispatch(NUM_PARTICLES,accumWgSize);
//    };

    float h = 0.3;
    float S = 0.1;

    // create hydrodynamics based acceleration function
    mpu::gph::ShaderProgram densityShader({{PROJECT_SHADER_PATH"Acceleration/hydrodynamics/naiveSPH-density.comp"}});
    densityShader.uniform1ui("num_of_particles",NUM_PARTICLES);
    densityShader.uniform1f("smoothing_length",h);
    densityShader.uniform1f("entropy",S);

    mpu::gph::ShaderProgram pressureShader({{PROJECT_SHADER_PATH"Acceleration/hydrodynamics/naiveSPH-pressureAcc.comp"}});
    pressureShader.uniform1ui("num_of_particles",NUM_PARTICLES);
    pressureShader.uniform1f("smoothing_length",h);

    uint32_t wgSize=calcWorkgroupSize(NUM_PARTICLES);
    auto accFunc = [densityShader,pressureShader,wgSize,pb](){
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        densityShader.dispatch(NUM_PARTICLES,wgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        pressureShader.dispatch(NUM_PARTICLES,wgSize);
    };

    //  create a simulator
    Leapfrog simulation(accFunc,pb,DT);
    simulation.start();

    float brightness=1;
    float size=PARTICLE_RENDER_SIZE;

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;


    // TODO: print particles to file
    // TODO: filnally code a f***ing gui
    // TODO: better accumulator
    // TODO: add gpu stopwatch
    // TODO: 2D mode
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

        // make size adjustable
        if(window.getKey(GLFW_KEY_KP_6) == GLFW_PRESS)
            size += size *0.5*dt;
        if(window.getKey(GLFW_KEY_KP_9) == GLFW_PRESS)
            size -= size *0.5*dt;
        renderer.setSize(size);

        // particle debug output
        if(window.getKey(GLFW_KEY_P) == GLFW_PRESS)
        {
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            std::vector<glm::vec2> hdata = pb.hydrodynamicsBuffer.read<glm::vec2>(10,0);

            logDEBUG("PARTICLE_HYDRO_DATA") << glm::to_string(hdata[1]) << "\n"
                                            << glm::to_string(hdata[2]) << "\n"
                                            << glm::to_string(hdata[3]) << "\n"
                                            << glm::to_string(hdata[4]) << "\n"
                                            << glm::to_string(hdata[5]) << "\n"
                                            << glm::to_string(hdata[6]) << "\n"
                                            << glm::to_string(hdata[7]) << "\n"
                                            << glm::to_string(hdata[8]) << "\n";
        }

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