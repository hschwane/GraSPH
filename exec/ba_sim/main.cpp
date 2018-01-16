#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>
#include <numeric>
#include <Timer/Stopwatch.h>

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
    ParticleBuffer pb(NUM_PARTICLES,ACCEL_THREADS_PER_PARTICLE,DENSITY_THREADS_PER_PARTICLE);
    ParticleSpawner spawner;
    spawner.setBuffer(pb);
//    spawner.spawnParticlesMultiSphere(TOTAL_MASS, {{1.5,{1.5,-.8,-.2},0.2},
//                                                   {1.5,{-1.5,-.5,-.3},0.2},
//                                                   {1.5,{0,1.5,-.5},0.2},
//                                                   {3,{0,0,0},0.4}});
    spawner.spawnParticlesSphere(TOTAL_MASS,TEMPERATURE,6);
    spawner.addRandomVelocityField(0.4, 0.35, 10152);
    spawner.addRandomVelocityField(0.1, 0.4, 98120);

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
    camera.setClip(0.1,200);

    float h = .4;
    float k = 0.05;
    float visc = 0.001;
    float sink_r = 0.0;
    float sink_th = 0.1;

    // create hydrodynamics based acceleration function
    uint32_t densityWgSize= 64;//calcWorkgroupSize(NUM_PARTICLES*THREADS_PER_PARTICLE);
    mpu::gph::ShaderProgram densityShader({{PROJECT_SHADER_PATH"Acceleration/sm-optimized/smo-SPHdensity.comp"}},
                                          {
                                            {"WGSIZE",{mpu::toString(densityWgSize)}},
                                            {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                            {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / densityWgSize / DENSITY_THREADS_PER_PARTICLE)}}
                                          });
    densityShader.uniform1f("smoothing_length",h);

    uint32_t wgSize=calcWorkgroupSize(NUM_PARTICLES);
    mpu::gph::ShaderProgram hydroAccum({{PROJECT_SHADER_PATH"Acceleration/hydroAccumulator.comp"}},
                                  {
                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                   {"HYDROS_PER_PARTICLE",{mpu::toString(DENSITY_THREADS_PER_PARTICLE)}}
                                  });
    hydroAccum.uniform1f("k",k);
    hydroAccum.uniform1f("sink_th",sink_th);

    uint32_t pressWgSize = 128;
    mpu::gph::ShaderProgram pressureShader({{PROJECT_SHADER_PATH"Acceleration/sm-optimized/smo-SPHpressureAccGravity.comp"}},
                                           {
                                                   {"WGSIZE",{mpu::toString(pressWgSize)}},
                                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                                   {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / pressWgSize / ACCEL_THREADS_PER_PARTICLE)}}
                                           });
    pressureShader.uniform1f("smoothing_length",h);
    pressureShader.uniform1f("visc",visc);
    pressureShader.uniform1f("gravity_constant",G);
    pressureShader.uniform1f("smoothing_epsilon_squared",EPS2);
    pressureShader.uniform1f("sink_r",sink_r);

    mpu::gph::ShaderProgram accAccum({{PROJECT_SHADER_PATH"Acceleration/accAccumulator.comp"}},
                                      {
                                       {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                       {"ACCELERATIONS_PER_PARTICLE",{mpu::toString(ACCEL_THREADS_PER_PARTICLE)}}
                                      });

    mpu::gph::ShaderProgram sinkHandler({{PROJECT_SHADER_PATH"Acceleration/sinkHandler.comp"}});

    auto accFunc = [densityShader,pressureShader,wgSize,densityWgSize,pressWgSize,hydroAccum,accAccum,sinkHandler](){
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/densityWgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        hydroAccum.dispatch(NUM_PARTICLES,wgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        pressureShader.dispatch(NUM_PARTICLES*ACCEL_THREADS_PER_PARTICLE/pressWgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        accAccum.dispatch(NUM_PARTICLES,wgSize);
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
    double simulationTime = dt;

    // TODO: change sph kernel
    // TODO: adaptive smoothing length
    // TODO: better viscosity
    // TODO: add sink particles
    // TODO: test fractation EOS
    // TODO: adaptive timestep
    // TODO: make the initial velocity noise better

    // TODO: add gpu stopwatch
    // TODO: print particles to file
    // TODO: filnally code a f***ing gui
    // TODO: better accumulator
    // TODO: 2D mode
    // TODO: spawn orbiting particles

    bool runSim = false;
    bool buttonDown = false;
    while( window.update())
    {
        dt = timer.getDeltaTime();
        camera.update(dt);

        if(window.getKey(GLFW_KEY_3) != GLFW_PRESS)
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
            std::vector<glm::vec2> hdata = pb.hydrodynamicsBuffer.read<glm::vec2>(pb.size(),0);

            glm::vec2 sum = std::accumulate( hdata.begin(), hdata.end(),glm::vec2(0,0));
            sum /= pb.size();

            logDEBUG("Particle data") << "Mean density: " << sum.y << " Mean Pressure: " << sum.x;
        }

        // run the simulation
        if(window.getKey(GLFW_KEY_1) == GLFW_PRESS && window.getKey(GLFW_KEY_2) == GLFW_RELEASE)
            runSim = true;
        else if(window.getKey(GLFW_KEY_2) == GLFW_PRESS)
            runSim = false;

        if(runSim)
        {
            simulation.advanceTime();
            lag += DT;
            simulationTime += DT;
        }

        // render the particles
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        renderer.draw();

        // performance display
        nbframes++;
        elapsedPerT += dt;
        if(elapsedPerT >= 2.0)
        {
            printf("%f ms/frame -- %f fps -- %f simSpeed -- %f simTime\n", 1000.0*elapsedPerT/double(nbframes), nbframes/elapsedPerT, lag/elapsedPerT, simulationTime);
            nbframes = 0;
            elapsedPerT = 0;
            lag = 0;
        }
    }

    return 0;
}