#include <Log/Log.h>
#include <Log/ConsoleSink.h>
#include <Timer/DeltaTimer.h>
#include <Graphics/Graphics.h>

#include "Common.h"
#include "ParticleSpawner.h"
#include "ParticleRenderer.h"
#include "DEsolver/SimpleDEsolver.h"
#include "DEsolver/Leapfrog.h"

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

//    std::vector<Particle> ptls;
//    Particle a;
//    a.position = {0.5,0,0,1};
//    a.velocity = {0,0,0,0};
//    a.acceleration = {0,0,0,0};
//    a.renderSize = 0.008;
//    a.mass=0.25;
//    ptls.push_back(a);
//    a.position = {-0.5,0,0,1};
//    ptls.push_back(a);
//    mpu::gph::Buffer pb(ptls);
//    pb.bindBase(PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);

    // create a renderer
    ParticleRenderer renderer;
    renderer.setViewportSize({WIDTH,HEIGHT});
    renderer.configureArrays(mpu::gph::offset_of(&Particle::position), mpu::gph::offset_of(&Particle::renderSize));
    renderer.setParticleBuffer<Particle>( pb, NUM_PARTICLES);

    // create camera
    mpu::gph::Camera camera(&window);
    camera.setMVP(&renderer);

    // create shaders for acceleration
    mpu::gph::ShaderProgram accShader({{PROJECT_SHADER_PATH"Acceleration/naive-gravity.comp"}});
    accShader.uniform1f("smoothing_epsilon_squared",  EPS2);
    accShader.uniform1f("gravity_constant",  G);
    accShader.uniform1ui("num_of_particles",  NUM_PARTICLES);

    uint32_t wgSize = calcWorkgroupSize(NUM_PARTICLES);
    auto accFunc = [accShader,wgSize](){
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        accShader.dispatch(NUM_PARTICLES,wgSize);
    };

    //  create a simulator
    Leapfrog simulation(accFunc,pb,NUM_PARTICLES,DT);
    simulation.start();

    // verlet
//    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"DEsolver/verlet.comp"}});
//    mpu::gph::ShaderProgram verletFirstShader({{PROJECT_SHADER_PATH"DEsolver/verletFirstStep.comp"}});
//    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    verletFirstShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    integShader.uniform1f("dt",DT);
//    verletFirstShader.uniform1f("dt",DT);
//    mpu::gph::Buffer verletBuffer;
//    verletBuffer.allocate<glm::vec4>(NUM_PARTICLES);
//    verletBuffer.bindBase( VERLET_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    accShader.dispatch(NUM_PARTICLES,100);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    verletFirstShader.dispatch(NUM_PARTICLES,100);

    // velocity verlet
//    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"DEsolver/velocityVerlet.comp"}});
//    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    integShader.uniform1f("dt",DT);
//    integShader.uniform1f("vel_dt",0.0f);
//    mpu::gph::Buffer verletBuffer;
//    verletBuffer.allocate<glm::vec4>(NUM_PARTICLES);
//    verletBuffer.bindBase( VERLET_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    accShader.dispatch(NUM_PARTICLES,100);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//    integShader.dispatch(NUM_PARTICLES,100);
//    integShader.uniform1f("vel_dt",DT);

    // RK2-Midpoint
//    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"DEsolver/RK2-Midpoint.comp"}});
//    mpu::gph::ShaderProgram rkM1Shader({{PROJECT_SHADER_PATH"DEsolver/rkIntermediate.comp"}});
//    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    rkM1Shader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    integShader.uniform1f("dt",DT);
//    rkM1Shader.uniform1f("dt",DT/2);
//    mpu::gph::Buffer rkM1Buffer;
//    rkM1Buffer.allocate<Particle>(NUM_PARTICLES);
//    rkM1Buffer.bindBase( RK_MX_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    // RK4
//    mpu::gph::ShaderProgram integShader({{PROJECT_SHADER_PATH"DEsolver/RK4.comp"}});
//    mpu::gph::ShaderProgram rkIntermediateShader({{PROJECT_SHADER_PATH"DEsolver/rkIntermediate.comp"}});
//    integShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    rkIntermediateShader.uniform1ui("num_of_particles",  NUM_PARTICLES);
//    integShader.uniform1f("dt",DT);
//
//    mpu::gph::Buffer rkM2Buffer;
//    rkM2Buffer.allocate<Particle>(NUM_PARTICLES);
//
//    mpu::gph::Buffer rkM3Buffer;
//    rkM3Buffer.allocate<Particle>(NUM_PARTICLES);
//
//    mpu::gph::Buffer rkM4Buffer;
//    rkM4Buffer.allocate<Particle>(NUM_PARTICLES);

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

            simulation.advanceTime();
            /*
            // calculate a(t,p(t),v(t))
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            accShader.dispatch(NUM_PARTICLES,500);

            // calculate v(t+dt/2) and a(t+dt/2) (m2)
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            pb.bindBase(RK_STATE_IN_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // this contains pm1(t) vm1(t) and am1(t,pm1(t),vm1(t))
            pb.bindBase(RK_DERIV_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
            rkM2Buffer.bindBase(RK_OUT_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // pm2(t+dt/2) and vm2(t+dt/2) will be stored here
            rkIntermediateShader.uniform1f("dt",DT/2);
            rkIntermediateShader.dispatch(NUM_PARTICLES,500); // calculate pm2(t+dt/2) using vm1(t)*dt/2, calculate vm2(t+dt/2) using am1(t,p(t),v(t))*dt/2

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            rkM2Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
            accShader.dispatch(NUM_PARTICLES,500);  // calculate am2(t+dt/2,pm2(t+dt/2),vm2(t+dt/2))

            // calculate v(t+dt/2) and a(t+dt/2) again, but using the derivatives from m2
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            rkM2Buffer.bindBase(RK_DERIV_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // this contains vm2(t+dt/2) and am2(t+dt/2,pm2(t+dt/2,vm2(t+dt/2)) (and pm2(t+dt/2))
            rkM3Buffer.bindBase(RK_OUT_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // pm3(t+dt/2) and vm3(t+dt/2) will be stored here
            rkIntermediateShader.dispatch(NUM_PARTICLES,500); // calculate pm3(t+dt/2) using vm2(t+dt/2)*dt/2, calculate v(t+dt/2) using am2(t+dt/2, pm2(t+dt/2), vm2(t+dt/2))*dt/2

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            rkM3Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
            accShader.dispatch(NUM_PARTICLES,500);  // calculate am3(t+dt/2,pm3(t+dt/2),vm3(t+dt/2))

            // calculate v(t+dt) and a(t+dt) using derivatives from m3
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            rkM3Buffer.bindBase(RK_DERIV_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // this contains vm3(t+dt/2) and am3(t+dt/2,pm3(t+dt/2),vm3(t+dt/2))  (and pm3(t+dt/2))
            rkM4Buffer.bindBase(RK_OUT_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // pm4(t+dt) and vm4(t+dt) will be stored here
            rkIntermediateShader.uniform1f("dt",DT);
            rkIntermediateShader.dispatch(NUM_PARTICLES,500); // calculate pm4(t+dt) using vm3(t+dt/2)*dt, calculate vm4(t+dt) using am3(t+dt/2,pm3(t+dt/2),vm3(t+dt/2))*dt

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            rkM4Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
            accShader.dispatch(NUM_PARTICLES,500);  // calculate am4(t+dt/2,pm4(t+dt/2),vm4(t+dt/2))

            // put everything together with proper weights
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            pb.bindBase(PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
            rkM2Buffer.bindBase( RK_MTWO_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
            rkM3Buffer.bindBase( RK_MTHREE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
            rkM4Buffer.bindBase( RK_MFOUR_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
            integShader.dispatch(NUM_PARTICLES,500);
*/
              // RK2
//            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//            accShader.dispatch(NUM_PARTICLES,500);
//            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//            rkM1Shader.dispatch(NUM_PARTICLES,500);
//            rkM1Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
//            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//            accShader.dispatch(NUM_PARTICLES,500);
//
//            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//            pb.bindBase( PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
//            rkM1Buffer.bindBase( RK_MX_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
//            integShader.dispatch(NUM_PARTICLES,500);

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