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

double DT = INITIAL_DT;

long double timeUnitInYears(const long double time)
{
    return sqrtl(powl(LENGTH_UNIT,3) / (MASS_UNIT * 6.674e-11l)) / (31556925.261) * time;
}

void printSimulationInfo()
{
    logINFO("Simulation") << "Simulating Gas cloud with mass of "
                          << MASS_UNIT * TOTAL_MASS / Ms
                          << " solar masses and diameter of "
                          << 2.0l * LENGTH_UNIT * SPAWN_RADIUS / au
                          << " AU. Total Volume: "
                          << 4.0l / 3.0l * M_PI * powl(LENGTH_UNIT * SPAWN_RADIUS, 3)
                          << " m^3. Initial density: "
                          << (MASS_UNIT * TOTAL_MASS) / (4.0l / 3.0l * M_PI * powl(LENGTH_UNIT * SPAWN_RADIUS, 3))
                          << " kg/(m^3). One time unit is "
                          << timeUnitInYears(1)
                          << " years.";
}

float getNextRefCubeSize()
{
    static struct
    {
        std::vector<std::pair<std::string, float>> sizes =
                {
                        {"Initial Cloud Size", 2*SPAWN_RADIUS},
                        {"1 pc", pc / LENGTH_UNIT},
                        {"100 000 AU", 100000 * au / LENGTH_UNIT},
                        {"10 000 AU", 10000 * au / LENGTH_UNIT},
                        {"1000 AU", 1000 * au / LENGTH_UNIT},
                        {"100 AU", 100 * au / LENGTH_UNIT},
                        {"10 AU", 10 * au / LENGTH_UNIT},
                        {"1 AU", au / LENGTH_UNIT},
                        {"1 internal unit",1},
                        {"invisible",0}
                };

        unsigned current{0};
    } refCubeSizes;

    refCubeSizes.current = (refCubeSizes.current +1) % refCubeSizes.sizes.size();

    float size = refCubeSizes.sizes[refCubeSizes.current].second;
    logINFO("Reference") << "Reference cube set to " << refCubeSizes.sizes[refCubeSizes.current].first << " (" << LENGTH_UNIT * size / au<< " AU, " << LENGTH_UNIT * size << " m, " << size << " internal units)";
    return size;
}

static const std::vector<GLfloat> cube =
{
        -0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,
        -0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f, 0.5f,

        0.5f,0.5f,0.5f,
        0.5f,0.5f,-0.5f,
        0.5f,0.5f,0.5f,
        0.5f,-0.5f,0.5f,
        0.5f,0.5f,0.5f,
        -0.5f,0.5f, 0.5f,

        0.5f,-0.5f,-0.5f,
        0.5f,0.5f,-0.5f,
        0.5f,-0.5f,-0.5f,
        0.5f,-0.5f,0.5f,

        -0.5f,-0.5f,0.5f,
        0.5f,-0.5f,0.5f,

        -0.5f,0.5f,-0.5f,
         0.5f,0.5f,-0.5f,

        -0.5f,0.5f,0.5f,
        -0.5f,-0.5f,0.5f,
        -0.5f,0.5f,0.5f,
        -0.5f,0.5f,-0.5f,
};

int main()
{
    // initialise log
    mpu::Log mainLog(mpu::DEBUG, mpu::ConsoleSink());

    // create window and init gl
    mpu::gph::Window window(WIDTH,HEIGHT,"Star Formation Sim");

    // add the shader include pathes
    mpu::gph::addShaderIncludePath(LIB_SHADER_PATH);
    mpu::gph::addShaderIncludePath(PROJECT_SHADER_PATH);

    // set some gl options
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.f);
    mpu::gph::enableVsync(false);

    // set up a reference cube
    mpu::gph::Buffer refCubeVert(cube);
    mpu::gph::VertexArray refCube;
    refCube.addAttributeBufferArray(0,refCubeVert,0,3*sizeof(GLfloat),3,0);
    mpu::gph::ShaderProgram cubeRefShader({ {LIB_SHADER_PATH"simple.frag"},
                                        {LIB_SHADER_PATH"simple.vert"} });
    cubeRefShader.uniform4f("color",REFBOX_COLOR);

    // generate some particles
    ParticleBuffer pb(NUM_PARTICLES,ACCEL_THREADS_PER_PARTICLE,DENSITY_THREADS_PER_PARTICLE);
    ParticleSpawner spawner;
    spawner.setBuffer(pb);
    spawner.spawnParticlesSphere(TOTAL_MASS,SPAWN_RADIUS, INITIAL_H);

//    spawner.addMultiFrequencyCurl( {
//                                           {{0.9},{0.1}},
//                                           {{0.6},{0.3}},
//                                           {{0.4},{0.3}},
//                                           {{0.3},{0.6}},
//                                   },1612,HMIN,HMAX,TOTAL_MASS / NUM_PARTICLES);
    spawner.addAngularVelocity({0,0.12f,0});


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
    camera.setClip(0.0001,200);
    camera.setPosition({0,0, 2.5 * SPAWN_RADIUS});


    // compile and confiure all the shader
    mpu::gph::ShaderProgram adjustH({{PROJECT_SHADER_PATH"Simulation/calculateH.comp"}});
    adjustH.uniform1f("hmin",HMIN);
    adjustH.uniform1f("hmax",HMAX);
    adjustH.uniform1f("mass_per_particle", TOTAL_MASS / NUM_PARTICLES);
    adjustH.uniform1f("num_neighbours",NUM_NEIGHBOURS);

    mpu::gph::ShaderProgram densityShader({{PROJECT_SHADER_PATH"Simulation/calculateDensity.comp"}},
                                          {
                                            {"WGSIZE",{mpu::toString(DENSITY_WGSIZE)}},
                                            {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                            {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / DENSITY_WGSIZE / DENSITY_THREADS_PER_PARTICLE)}}
                                          });

    mpu::gph::ShaderProgram hydroAccum({{PROJECT_SHADER_PATH"Simulation/densityAccumulator.comp"}},
                                  {
                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                   {"HYDROS_PER_PARTICLE",{mpu::toString(DENSITY_THREADS_PER_PARTICLE)}}
                                  });
    hydroAccum.uniform1f("a",A);
    hydroAccum.uniform1f("ac1",AC1);
    hydroAccum.uniform1f("ac2",AC2);
    hydroAccum.uniform1f("frag_limit",FRAG_LIMIT);

    mpu::gph::ShaderProgram pressureShader({{PROJECT_SHADER_PATH"Simulation/calculateAcceleration.comp"}},
                                           {
                                                   {"WGSIZE",{mpu::toString(PRESSURE_WGSIZE)}},
                                                   {"NUM_PARTICLES",{mpu::toString(NUM_PARTICLES)}},
                                                   {"TILES_PER_THREAD",{mpu::toString(NUM_PARTICLES / PRESSURE_WGSIZE / ACCEL_THREADS_PER_PARTICLE)}}
                                           });
    pressureShader.uniform1f("alpha",VISC);
    pressureShader.uniform1f("eps_factor2",EPS_FACTOR*EPS_FACTOR);
    pressureShader.uniform1f("balsara_strength",BALSARA_STRENGTH);

    mpu::gph::ShaderProgram integrator({{PROJECT_SHADER_PATH"Simulation/integrateLeapfrog.comp"}},
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


    // group shader dispatches into useful functions
    auto findSml = [densityShader,hydroAccum,adjustH](int iterations)
    {
        for(int i=0; i<iterations; i++)
        {
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            hydroAccum.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            adjustH.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
        }
    };

    auto startSimulation = [densityShader,pressureShader,hydroAccum,integrator,adjustH]()
    {

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        hydroAccum.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        pressureShader.dispatch(NUM_PARTICLES*ACCEL_THREADS_PER_PARTICLE/PRESSURE_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        integrator.uniform1f("not_first_step",1);
        integrator.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
        integrator.uniform1f("not_first_step",1);
    };

    auto simulate = [densityShader,pressureShader,hydroAccum,integrator,adjustH]()
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        adjustH.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        densityShader.dispatch(NUM_PARTICLES*DENSITY_THREADS_PER_PARTICLE/DENSITY_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        hydroAccum.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        pressureShader.dispatch(NUM_PARTICLES*ACCEL_THREADS_PER_PARTICLE/PRESSURE_WGSIZE);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        integrator.dispatch(NUM_PARTICLES,GENERAL_WGSIZE);
    };

    findSml(20);
    startSimulation();

    printSimulationInfo();

    float brightness=PARTICLE_BRIGHTNESS;
    float size=PARTICLE_RENDER_SIZE;
    float referenceCubeSize=2*SPAWN_RADIUS;
    glm::vec3 refCubePos{0,0,0};
    float refCubeSpeed = 0.2;

    // timing
    mpu::DeltaTimer timer;
    double dt;
    int nbframes =0;
    double elapsedPerT = 0;

    double lag = 0;
    double simulationTime = DT;

    double newDT = DT;

    bool runSim = false;
    bool readyToPrint=true;
    bool readyToChangeRef=true;
    bool printRefcubeSize=false;
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

        // change refernce cube size
        if(window.getKey(GLFW_KEY_C) == GLFW_PRESS && readyToChangeRef)
        {
            readyToChangeRef = false;
            referenceCubeSize = getNextRefCubeSize();
        }
        else if(window.getKey(GLFW_KEY_C) == GLFW_RELEASE)
            readyToChangeRef = true;

        if(window.getKey(GLFW_KEY_L))
            refCubePos.x += dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_J))
            refCubePos.x -= dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_I))
            refCubePos.z -= dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_K))
            refCubePos.z += dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_O))
            refCubePos.y += dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_U))
            refCubePos.y -= dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_B))
            refCubePos = {0,0,0};

        if(window.getKey(GLFW_KEY_Y))
        {
            referenceCubeSize += dt * referenceCubeSize * refCubeSpeed;
            printRefcubeSize = true;
        }
        else if(window.getKey(GLFW_KEY_H))
        {
            referenceCubeSize -= dt * referenceCubeSize * refCubeSpeed;
            printRefcubeSize = true;
        }
        else if(printRefcubeSize)
        {
            logINFO("Reference") << "Reference cube set to " << referenceCubeSize * LENGTH_UNIT / au << " AU";
            printRefcubeSize = false;
        }

        if(window.getKey(GLFW_KEY_M))
            refCubeSpeed += dt * refCubeSpeed;
        if(window.getKey(GLFW_KEY_N))
            refCubeSpeed -= dt * refCubeSpeed;

        // run the simulation
        if(window.getKey(GLFW_KEY_1) == GLFW_PRESS && window.getKey(GLFW_KEY_2) == GLFW_RELEASE)
            runSim = true;
        else if(window.getKey(GLFW_KEY_2) == GLFW_PRESS)
            runSim = false;

        if(window.getKey(GLFW_KEY_P) == GLFW_PRESS && readyToPrint)
        {
            readyToPrint = false;
            auto nbs = pb.balsaraBuffer.read<glm::vec4>(pb.size(),0);

            int i = 0;
            float mean = 0;
            while(i<pb.size())
            {
                logINFO("debug_print") << "balsara switch of paticle " << i  << " is " << nbs[i].x;
                mean += nbs[i].x;
                i++;
            }
            mean = mean / pb.size();
            logINFO("debug_print") << "mean balsara switch: " << mean;
        }
        else if(window.getKey(GLFW_KEY_P) == GLFW_RELEASE)
            readyToPrint=true;

        mpu::gph::Buffer temp;
        temp.allocate<float>(pb.size(),GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
        pb.timestepBuffer.copyTo(temp);
        std::vector<float> dtdata = temp.read<float>( pb.size(),0);
        float desiredMaxDT = *std::min(dtdata.begin(),dtdata.end());

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
        }

        // render the particles
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        renderer.draw();

        // render the reference cube
        refCube.bind();
        cubeRefShader.use();
        cubeRefShader.uniformMat4("model_view_projection", renderer.getViewProjection() * glm::scale(glm::translate(glm::mat4(),refCubePos),glm::vec3(referenceCubeSize)));
        glDrawArrays(GL_LINES, 0, cube.size());

        // performance display
        nbframes++;
        elapsedPerT += dt;
        if(elapsedPerT >= PERFORMANCE_DISPLAY_INT)
        {
            std::cout << 1000.0*elapsedPerT/double(nbframes) << " ms/frame -- "
                      << nbframes/elapsedPerT << " fps -- "
                      << timeUnitInYears(simulationTime) << " simulated years -- "
                      << timeUnitInYears(lag)/elapsedPerT << " years/second -- "
                      << timeUnitInYears(lag)/nbframes << " years/frame -- "
                      << simulationTime << " internal time -- "
                      << lag/elapsedPerT << " speed -- "
                      << lag/nbframes << " average dt"
                      << std::endl;
            nbframes = 0;
            elapsedPerT = 0;
            lag = 0;
        }
    }

    return 0;
}