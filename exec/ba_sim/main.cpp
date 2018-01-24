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
//    spawner.spawnParticlesMultiSphere(TOTAL_MASS, {{1.5,{1.5,-.8,-.2},0.2},
//                                                   {1.5,{-1.5,-.5,-.3},0.2},
//                                                   {1.5,{0,1.5,-.5},0.2},
//                                                   {3,{0,0,0},0.4}});
    spawner.spawnParticlesSphere(TOTAL_MASS,SPAWN_RADIUS);
    spawner.addRandomVelocityField(0.4, 0.25, 512);
    spawner.addRandomVelocityField(0.1, 0.3, 1024);

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
    mpu::gph::ShaderProgram densityShader({{PROJECT_SHADER_PATH"Acceleration/sm-optimized/smo-SPHdensity.comp"}},
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

    mpu::gph::ShaderProgram pressureShader({{PROJECT_SHADER_PATH"Acceleration/sm-optimized/smo-SPHpressureAccGravity.comp"}},
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

    // sph
    // TODO: play with different h adjustment algorithmens
    // TODO: change sph kernel
    // TODO: add shear viscosity term to viscosity
    // TODO: test fractation EOS

    // initial conditions
    // TODO: make the initial velocity noise better
    // TODO: spawn orbiting particles

    // performance
    // TODO: adaptive timestep
    // TODO: enable prerenderd simulation
    // TODO: add datastructure
    // TODO: better accumulator

    // output and visualisation
    // TODO: output information of stars
    // TODO: make variable particle sizes possible
    // TODO: make star visualisation better

    // usability and debugging
    // TODO: add gpu stopwatch
    // TODO: print particles to readable file for debug
    // TODO: finally code a f***ing gui
    // TODO: better speed control for fast simulations

    // misc
    // TODO: fix the time integration scheme
    // TODO: make different particle sizes possible
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

            for(auto &&item : hdata)
            {
                logDEBUG("Particle data") << "Hydro: " << glm::to_string(item);
            }
            logDEBUG("Particle data") << "Mean density: " << sum.y << " Mean Pressure: " << sum.x << " Mean Smoothing length: " << sum.z;

            mpu::gph::Buffer temp;
            temp.allocate<glm::vec4>(pb.size(),GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
            pb.positionBuffer.copyTo(temp);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
            std::vector<glm::vec4> pdata = temp.read<glm::vec4>(pb.size(),0);
            for(auto &&item : pdata)
//            {
//                logDEBUG("Particle data") << "Position: " << glm::to_string(item);
//            }

            std::vector<glm::vec4> adata = pb.accelerationBuffer.read<glm::vec4>( pb.size()*pb.accPerParticle(),0);
            int i=0;
//            for(auto &&item : adata)
//            {
//                if(item.w != 0)
//                logWARNING("Particle data") << "ITS FUCKIN NOT ZERO! value is: " << item.w << " at entry " << i;
//                i++;
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