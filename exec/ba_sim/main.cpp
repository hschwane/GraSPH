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
#include "Settings.h"

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
    spawner.spawnParticlesSphere(TOTAL_MASS,SPAWN_RADIUS);
    spawner.addMultiFrequencyCurl( {
//                                        {{0.6},{0.2}},
                                        {{0.4},{0.6}},
                                        {{0.3},{1}},
                                        {{0.1},{0.8}}
//                                        {{0.08},{0.8}}
                                       },74,HMIN,HMAX);
//    spawner.addSimplexVelocityField(0.5,0.15,10);
//    spawner.addSimplexVelocityField(0.3,0.05,452);
//    spawner.addSimplexVelocityField(0.1,0.15,876);


    // create a renderer
    ParticleRenderer renderer;
    renderer.setParticleBuffer(pb);
    renderer.setShaderSettings(Falloff::LINEAR);
    renderer.enableAdditiveBlending(true);
    renderer.enableDepthTest(false);
    renderer.setViewportSize({WIDTH,HEIGHT});
    renderer.setColor(PARTICLE_COLOR);
    renderer.setBrightness(PARTICLE_BRIGHTNESS);
    renderer.setSize(PARTICLE_RENDER_SIZE);

    // create camera
    mpu::gph::Camera camera(std::make_shared<mpu::gph::SimpleWASDController>(&window,10,4));
    camera.setMVP(&renderer);
    camera.setClip(0.01,200);

    // create hydrodynamics based acceleration function
    mpu::gph::ShaderProgram densityShader({{PROJECT_SHADER_PATH"Acceleration/smo-SPHdensity.comp"}},
                                          {
                                            {"WGSIZE",{mpu::toString(DENSITY_WGSIZE)}},
                                            {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                            {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / DENSITY_WGSIZE / DENSITY_THREADS_PER_PARTICLE)}}
                                          });
    densityShader.uniform1f("eps",HEPS);

    uint32_t wgSize=calcWorkgroupSize(NUM_PARTICLES);
    mpu::gph::ShaderProgram hydroAccum({{PROJECT_SHADER_PATH"Acceleration/hydroAccumulator.comp"}},
                                  {
                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                   {"HYDROS_PER_PARTICLE",{mpu::toString(DENSITY_THREADS_PER_PARTICLE)}}
                                  });
    hydroAccum.uniform1f("sink_th",SINK_TH);
    hydroAccum.uniform1f("k",K);
    hydroAccum.uniform1f("ac",AC);

    mpu::gph::ShaderProgram pressureShader({{PROJECT_SHADER_PATH"Acceleration/smo-SPHpressureAccGravity.comp"}},
                                           {
                                                   {"WGSIZE",{mpu::toString(PRESSURE_WGSIZE)}},
                                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                                   {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / PRESSURE_WGSIZE / ACCEL_THREADS_PER_PARTICLE)}}
                                           });
    pressureShader.uniform1f("alpha",VISC);
    pressureShader.uniform1f("gravity_constant",G);
    pressureShader.uniform1f("smoothing_epsilon_squared",EPS2);
    pressureShader.uniform1f("smoothing_epsilon_squared_sph",EPS2_SPH);
    pressureShader.uniform1f("sink_r",SINK_R);
    pressureShader.uniform1f("dt",DT);
    pressureShader.uniform1f("k",K);
    pressureShader.uniform1f("ac",AC);

    mpu::gph::ShaderProgram accAccum({{PROJECT_SHADER_PATH"Acceleration/accAccumulator.comp"}},
                                      {
                                       {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                       {"ACCELERATIONS_PER_PARTICLE",{mpu::toString(ACCEL_THREADS_PER_PARTICLE)}}
                                      });

    mpu::gph::ShaderProgram adjustH({{PROJECT_SHADER_PATH"Acceleration/adjustH.comp"}});
    adjustH.uniform1f("hmin",HMIN);
    adjustH.uniform1f("hmax",HMAX);

    mpu::gph::ShaderProgram sinkHandler({{PROJECT_SHADER_PATH"Acceleration/sinkHandler.comp"}});

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);

    auto accFunc = [densityShader,pressureShader,wgSize,hydroAccum,accAccum,adjustH](){
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        adjustH.dispatch(NUM_PARTICLES,wgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        hydroAccum.dispatch(NUM_PARTICLES,wgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        pressureShader.dispatch(NUM_PARTICLES*ACCEL_THREADS_PER_PARTICLE/PRESSURE_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        accAccum.dispatch(NUM_PARTICLES,wgSize);
//        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//        sinkHandler.dispatch(NUM_PARTICLES,wgSize);
    };

    //  create a simulator
    Leapfrog simulation(accFunc,pb,DT);
    simulation.start();

    float brightness=PARTICLE_BRIGHTNESS;
    float size=PARTICLE_RENDER_SIZE;

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;
    double simulationTime = DT;

    // sink particles and particle merging
    // TODO: make sink particles use impulse
    // TODO: make sink paricle better interact with each other
    // TODO: make sink particles spawning more realistic
    // TODO: add sph border conditions at  sink particles
    // TODO: maybe optimize tagging and make absorption faster

    // initial conditions
    // TODO: add rotational velocity

    // gravity
    // TODO: use sph softening method instead of plummer sphere

    // sph
    // TODO: change sph kernel
    // TODO: test fractation EOS

    // performance
    // TODO: adaptive timestep and fix the time integration scheme
    // TODO: enable prerenderd simulation
    // TODO: add datastructure

    // output and visualisation
    // TODO: output information of stars
    // TODO: make variable particle sizes possible
    // TODO: make star visualisation better

    // usability and debugging
    // TODO: class tp generate different simulation settings (fast mode usw...)
    // TODO: print particles to readable file for debug
    // TODO: finally code a f***ing gui
    // TODO: better speed control for fast simulations <- haha not needed, all the simulations are slow as f**k

    // not so important stuff
    // TODO: add gpu stopwatch
    // TODO: spawn orbiting particles
    // TODO: better accumulator
    // TODO: 2D mode


    bool runSim = false;
    bool printButtonDown = false;
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
        if(window.getKey(GLFW_KEY_P) == GLFW_PRESS && !printButtonDown)
        {
            printButtonDown = true;
            glFinish();
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            std::vector<glm::vec4> hdata = pb.hydrodynamicsBuffer.read<glm::vec4>(pb.size(),0);

            glm::vec4 sum = std::accumulate( hdata.begin(), hdata.end(),glm::vec4(0));
            sum /= hdata.size();

//            for(auto &&item : hdata)
//            {
//                logDEBUG("Particle data") << "Hydro: " << glm::to_string(item);
//            }
            logDEBUG("Particle data") << "Mean density: " << sum.y << " Mean Pressure: " << sum.x << " Mean Smoothing length: " << sum.z;

            mpu::gph::Buffer temp;
            temp.allocate<glm::vec4>(pb.size(),GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
            pb.positionBuffer.copyTo(temp);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            std::vector<glm::vec4> pdata = temp.read<glm::vec4>(pb.size(),0);
//            for(auto &&item : pdata)
//            {
//                logDEBUG("Particle data") << "Position: " << glm::to_string(item);
//            }

            temp.recreate();
            temp.allocate<float>(pb.size(),GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
            pb.smlengthBuffer.copyTo(temp);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            std::vector<float> smlength = temp.read<float>(pb.size(),0);
//            int i=0;
//            for(auto &&item : hdata)
//            {
//                logDEBUG("Particle data") << "mass under curve: " << 4 * glm::pi<float>() /3 * smlength[i]*smlength[i]*smlength[i] * item.y << " H:" << smlength[i];
//                i++;
//            }

//            std::vector<glm::vec4> adata = pb.accelerationBuffer.read<glm::vec4>( pb.size()*pb.accPerParticle(),0);
//            int i=0;
//            for(auto &&item : adata)
//            {
//                if(i < 600)
//                logDEBUG("Particle data") << "acc is: " << glm::to_string(item) << " at entry " << i;
//                i++;
//            }

            std::vector<glm::vec4> vdata = pb.velocityBuffer.read<glm::vec4>( pb.size(),0);
            for(auto &&item : vdata)
            {
                    logDEBUG("Particle data") << "vel is: " << glm::to_string(item);
            }

            glm::vec4 pos = std::accumulate( pdata.begin(), pdata.end(),glm::vec4(0,0,0,0));
            logDEBUG("Particle data") << "total mass: " << pos.w;
        }
        else if(window.getKey(GLFW_KEY_P) == GLFW_RELEASE)
            printButtonDown = false;

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