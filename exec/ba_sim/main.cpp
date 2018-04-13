#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>
#include <numeric>
#include <Timer/Stopwatch.h>

#include "Common.h"
#include "ParticleSpawner.h"
#include "ParticleRenderer.h"
#include "Settings.h"

constexpr int HEIGHT = 800;
constexpr int WIDTH = 800;

double DT = INITIAL_DT;

unsigned int NUM_PARTICLES    = 4096;
unsigned int DENSITY_THREADS_PER_PARTICLE = 0;
unsigned int ACCEL_THREADS_PER_PARTICLE   = 0;
unsigned int DENSITY_WGSIZE               = 0;
unsigned int PRESSURE_WGSIZE              = 0;

unsigned int lastTotalFrames = 0;

void spawnParticles(ParticleBuffer pb)
{
    // generate some particles
    ParticleSpawner spawner;
    spawner.setBuffer(pb);
    spawner.spawnParticlesSphere(TOTAL_MASS,SPAWN_RADIUS, INITIAL_H);

    spawner.addMultiFrequencyCurl( {
                                           {{0.9},{0.1}},
                                           {{0.6},{0.3}},
                                           {{0.4},{0.3}},
                                           {{0.3},{0.6}},
//                                        {{0.1},{0.2}}
                                   },1612,HMIN,HMAX,TOTAL_MASS / NUM_PARTICLES);
//    spawner.addSimplexVelocityField(0.8,0.05,42);
//    spawner.addSimplexVelocityField(0.6,0.05,452);
//    spawner.addSimplexVelocityField(0.1,0.15,876);

//    spawner.addCurlVelocityField(0.5,0.1,1111);

    spawner.addAngularVelocity({0,0.12f,0});
}

double run()
{
    // create window and init gl
    mpu::gph::Window window(WIDTH,HEIGHT,"Star Formation Sim");

    // add the shader include pathes
    mpu::gph::addShaderIncludePath(LIB_SHADER_PATH);
    mpu::gph::addShaderIncludePath(PROJECT_SHADER_PATH);

    // set some gl options
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.f);
    mpu::gph::enableVsync(false);

    ParticleBuffer pb(NUM_PARTICLES,ACCEL_THREADS_PER_PARTICLE,DENSITY_THREADS_PER_PARTICLE);
    spawnParticles(pb);

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
    densityShader.uniform1f("heps_factor",HEPS_FACTOR);

    uint32_t wgSize=calcWorkgroupSize(NUM_PARTICLES);
    mpu::gph::ShaderProgram hydroAccum({{PROJECT_SHADER_PATH"Acceleration/hydroAccumulator.comp"}},
                                  {
                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                   {"HYDROS_PER_PARTICLE",{mpu::toString(DENSITY_THREADS_PER_PARTICLE)}}
                                  });
    hydroAccum.uniform1f("k",K);
    hydroAccum.uniform1f("ac1",AC1);
    hydroAccum.uniform1f("ac2",AC2);
    hydroAccum.uniform1f("frag_limit",FRAG_LIMIT);

    mpu::gph::ShaderProgram pressureShader({{PROJECT_SHADER_PATH"Acceleration/smo-SPHpressureAccGravity.comp"}},
                                           {
                                                   {"WGSIZE",{mpu::toString(PRESSURE_WGSIZE)}},
                                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                                   {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / PRESSURE_WGSIZE / ACCEL_THREADS_PER_PARTICLE)}}
                                           });
    pressureShader.uniform1f("alpha",VISC);
    pressureShader.uniform1f("gravity_constant",G);
    pressureShader.uniform1f("eps_factor2",EPS_FACTOR*EPS_FACTOR);
    pressureShader.uniform1f("k",K);

    mpu::gph::ShaderProgram integrator({{PROJECT_SHADER_PATH"Acceleration/integrator.comp"}},
                                      {
                                       {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                       {"ACCELERATIONS_PER_PARTICLE",{mpu::toString(ACCEL_THREADS_PER_PARTICLE)}}
                                      });
    integrator.uniform1f("dt",DT);
    integrator.uniform1f("next_dt",DT);
    integrator.uniform1f("not_first_step",0);
    integrator.uniform1f("eps_factor",EPS_FACTOR);
    integrator.uniform1f("gravity_accuracy",GRAV_ACCURACY);
    integrator.uniform1f("courant_number",COURANT_NUMBER);


    mpu::gph::ShaderProgram adjustH({{PROJECT_SHADER_PATH"Acceleration/adjustH.comp"}});
    adjustH.uniform1f("hmin",HMIN);
    adjustH.uniform1f("hmax",HMAX);
    adjustH.uniform1f("mass_per_particle", TOTAL_MASS / NUM_PARTICLES);
    adjustH.uniform1f("num_neighbours",NUM_NEIGHBOURS);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);

    auto simulate = [densityShader,pressureShader,wgSize,hydroAccum,integrator,adjustH](){
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        adjustH.dispatch(NUM_PARTICLES,wgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        hydroAccum.dispatch(NUM_PARTICLES,wgSize);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        pressureShader.dispatch(NUM_PARTICLES*ACCEL_THREADS_PER_PARTICLE/PRESSURE_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        integrator.dispatch(NUM_PARTICLES,wgSize);
    };

    simulate();
    integrator.uniform1f("not_first_step",1);


    float brightness=PARTICLE_BRIGHTNESS;
    float size=PARTICLE_RENDER_SIZE;

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;
    double simulationTime = DT;

    double newDT = DT;

    // sink particles and particle merging
    // TODO: make sink particles use impulse
    // TODO: make sink paricle better interact with each other
    // TODO: make sink particles spawning more realistic
    // TODO: add sph border conditions at  sink particles
    // TODO: maybe optimize tagging and make absorption faster

    // performance
    // TODO: enable prerenderd simulation
    // TODO: add datastructure

    // output and visualisation
    // TODO: output information of stars
    // TODO: make star visualisation better

    // usability and debugging
    // TODO: print particles to readable file for debug
    // TODO: finally code a gui

    // not so important stuff
    // TODO: make variable particle sizes possible / better gas rendering
    // TODO: class tp generate different simulation settings (fast mode usw...)
    // TODO: add gpu stopwatch
    // TODO: better accumulator
    // TODO: 2D mode

    double runningAverage = 0;
    int numberOfRuns = 0;

    long long framesTotalAverage = 0;
    long long framesTotal =1;

    bool runSim = true;
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

            glm::vec4 maxRho = *std::max_element(hdata.begin(), hdata.end(),[](const glm::vec4 &a, const glm::vec4 &b){ return(a.y < b.y);});
            glm::vec4 maxP = *std::max_element(hdata.begin(), hdata.end(),[](const glm::vec4 &a, const glm::vec4 &b){ return(a.y < b.y);});

//            for(auto &&item : hdata)
//            {
//                logDEBUG("Particle data") << "Hydro: " << glm::to_string(item);
//            }
            logDEBUG("Particle data") << "Mean density: " << sum.y << " Mean Pressure: " << sum.x;
            logDEBUG("Particle data") << "Max density: " << maxRho.y << " Max Pressure: " << maxP.x;

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
//            for(auto &&item : vdata)
//            {
//                    logDEBUG("Particle data") << "vel is: " << glm::to_string(item);
//            }

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

        mpu::gph::Buffer temp;
        temp.allocate<float>(pb.size(),GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
        pb.timestepBuffer.copyTo(temp);
        std::vector<float> dtdata = temp.read<float>( pb.size(),0);
        float desiredMaxDT = *std::min(dtdata.begin(),dtdata.end());

//        while(newDT < MAX_DT && newDT < desiredMaxDT)
//        {
//            newDT = newDT*2;
//        }
//
//        while(newDT > MIN_DT && newDT > desiredMaxDT)
//        {
//            newDT = newDT/2;
//        }
//        desiredMaxDT *=0.9;
        newDT = glm::clamp( desiredMaxDT, float(MIN_DT),float(MAX_DT));
        integrator.uniform1f("next_dt",newDT);

        if(runSim)
        {
            simulate();
            lag += DT;
            simulationTime += DT;
            if(newDT != DT)
            {
                DT = newDT;
                integrator.uniform1f("dt",DT);
            }

            framesTotal++;
            nbframes++;
            elapsedPerT += dt;
            if(simulationTime >= 3.5 || (simulationTime >= 0.5 && numberOfRuns == 0))
            {
                if(numberOfRuns !=0)
                {

                    logWARNING("TIMING") << "run " << numberOfRuns << " average time: "
                                         << 1000.0 * elapsedPerT / double(nbframes) << " total frames: " << framesTotal;
                    runningAverage += (1000.0 * elapsedPerT / double(nbframes) - runningAverage) / (numberOfRuns);
                    framesTotalAverage += (framesTotal - framesTotalAverage) / (numberOfRuns);

                    if (numberOfRuns >= 3)
                    {
//                    logWARNING("ERROR") << "5 runs average time: " << runningAverage;

                        lastTotalFrames = framesTotalAverage;
                        return runningAverage;
                    }
                }
                numberOfRuns++;

                spawnParticles(pb);

                DT = INITIAL_DT;
                dt=0;
                nbframes =0;
                elapsedPerT = 0;
                lag = 0;
                simulationTime = DT;
                newDT = DT;
                framesTotal =1;

                integrator.uniform1f("dt",DT);
                integrator.uniform1f("next_dt",DT);
                integrator.uniform1f("not_first_step",0);
                simulate();
                integrator.uniform1f("not_first_step",1);
            }
        }

        // render the particles
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        renderer.draw();

        // performance display
    }

    return 0;
}

int main()
{
    // initialise log
    mpu::Log mainLog(mpu::WARNING, mpu::ConsoleSink());


    std::ofstream outData("/home/hschwane/baTimingData_"+ mpu::toString(std::time(nullptr))+".csv");
    outData << "Star formation simulator timings\n";
    outData << "particle count, threads density, threads acc, wgsize, time in ms, total frames simulated\n";
    outData.flush();

    for(unsigned int i=4096; i<=40000; i=i*2) // particle count
    {
        for(unsigned int j=32; j<=512; j=j*2) // workgroup size
        {
            for (unsigned int x = 1; x <= 128; x = x * 2)
            {
                for (unsigned int y = 1; y <= x; y = y * 2)
                {
                    NUM_PARTICLES = i;
                    DENSITY_WGSIZE = j;
                    PRESSURE_WGSIZE = j;
                    ACCEL_THREADS_PER_PARTICLE = y;
                    DENSITY_THREADS_PER_PARTICLE = x;

                    if( NUM_PARTICLES % (DENSITY_THREADS_PER_PARTICLE*DENSITY_WGSIZE) != 0 || NUM_PARTICLES % (ACCEL_THREADS_PER_PARTICLE*PRESSURE_WGSIZE) !=0)
                        continue;

                    double time = run();

                    logERROR("TIMING") << NUM_PARTICLES << " particle, " << DENSITY_THREADS_PER_PARTICLE
                                       << " density threads, " << ACCEL_THREADS_PER_PARTICLE << " acc threads, " << j << " wgsize, " << time
                                       << " ms " << lastTotalFrames << " frames";
                    outData << NUM_PARTICLES << "," << DENSITY_THREADS_PER_PARTICLE << "," << ACCEL_THREADS_PER_PARTICLE
                            << "," << j << "," << time << "," << lastTotalFrames << "\n";
                    outData.flush();
                }
            }
        }
    }
}