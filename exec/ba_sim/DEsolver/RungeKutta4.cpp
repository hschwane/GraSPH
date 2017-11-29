/*
 * mpUtils
 * RungeKutta4.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the RungeKutta4 class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "RungeKutta4.h"
//--------------------

// function definitions of the RungeKutta4 class
//-------------------------------------------------------------------
RungeKutta4::RungeKutta4() : RungeKutta4(nullptr,mpu::gph::Buffer(),0,0)
{
}

RungeKutta4::RungeKutta4(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number,
                               double dt) : DEsolver(DEsolverFlag::REQUIRE_FOUR_ACC | DEsolverFlag::IS_SELF_STARTING),
                                            m_rk4Shader({{PROJECT_SHADER_PATH"/DEsolver/RK4.comp"}}),
                                            m_rkIntermediateShader({{PROJECT_SHADER_PATH"/DEsolver/rkIntermediate.comp"}}),
                                            m_calcAcceleration(accelerator), m_dt(dt),
                                            m_numParticles(number), m_wgSize(calcWorkgroupSize(number)),
                                            m_particleBuffer(particleBuffer)

{
    m_rk4Shader.uniform1f("dt",dt);
    m_particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_rkM2Buffer.allocate<Particle>(m_numParticles);
    m_rkM3Buffer.allocate<Particle>(m_numParticles);
    m_rkM4Buffer.allocate<Particle>(m_numParticles);
}

void RungeKutta4::setAccelerator(std::function<void(void)> accelerator)
{
    m_calcAcceleration = accelerator;
}

void RungeKutta4::setDT(double dt)
{
    m_rk4Shader.uniform1f("dt",dt);
    m_dt = dt;
}

void RungeKutta4::setParticles(mpu::gph::Buffer particleBuffer, uint32_t number)
{
    m_particleBuffer = particleBuffer;
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = number;
    m_wgSize = calcWorkgroupSize(number);

    m_rkM2Buffer.recreate();
    m_rkM2Buffer.allocate<Particle>(m_numParticles);
    m_rkM3Buffer.recreate();
    m_rkM3Buffer.allocate<Particle>(m_numParticles);
    m_rkM4Buffer.recreate();
    m_rkM4Buffer.allocate<Particle>(m_numParticles);
}

void RungeKutta4::advanceTime()
{
    // calculate a(t,p(t),v(t))
    m_calcAcceleration();

    // calculate v(t+dt/2) and a(t+dt/2) (m2)
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_particleBuffer.bindBase(RK_STATE_IN_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // this contains pm1(t) vm1(t) and am1(t,pm1(t),vm1(t))
    m_particleBuffer.bindBase(RK_DERIV_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
    m_rkM2Buffer.bindBase(RK_OUT_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // pm2(t+dt/2) and vm2(t+dt/2) will be stored here
    m_rkIntermediateShader.uniform1f("dt",m_dt/2);
    m_rkIntermediateShader.dispatch(m_numParticles,m_wgSize); // calculate pm2(t+dt/2) using vm1(t)*dt/2, calculate vm2(t+dt/2) using am1(t,p(t),v(t))*dt/2
    m_rkM2Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_calcAcceleration();  // calculate am2(t+dt/2,pm2(t+dt/2),vm2(t+dt/2))

    // calculate v(t+dt/2) and a(t+dt/2) again, but using the derivatives from m2
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_rkM2Buffer.bindBase(RK_DERIV_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // this contains vm2(t+dt/2) and am2(t+dt/2,pm2(t+dt/2,vm2(t+dt/2)) (and pm2(t+dt/2))
    m_rkM3Buffer.bindBase(RK_OUT_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // pm3(t+dt/2) and vm3(t+dt/2) will be stored here
    m_rkIntermediateShader.dispatch(m_numParticles,m_wgSize); // calculate pm3(t+dt/2) using vm2(t+dt/2)*dt/2, calculate v(t+dt/2) using am2(t+dt/2, pm2(t+dt/2), vm2(t+dt/2))*dt/2
    m_rkM3Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_calcAcceleration();  // calculate am3(t+dt/2,pm3(t+dt/2),vm3(t+dt/2))

    // calculate v(t+dt) and a(t+dt) using derivatives from m3
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_rkM3Buffer.bindBase(RK_DERIV_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // this contains vm3(t+dt/2) and am3(t+dt/2,pm3(t+dt/2),vm3(t+dt/2))  (and pm3(t+dt/2))
    m_rkM4Buffer.bindBase(RK_OUT_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER); // pm4(t+dt) and vm4(t+dt) will be stored here
    m_rkIntermediateShader.uniform1f("dt",m_dt);
    m_rkIntermediateShader.dispatch(m_numParticles,m_wgSize); // calculate pm4(t+dt) using vm3(t+dt/2)*dt, calculate vm4(t+dt) using am3(t+dt/2,pm3(t+dt/2),vm3(t+dt/2))*dt
    m_rkM4Buffer.bindBase( PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_calcAcceleration();  // calculate am4(t+dt/2,pm4(t+dt/2),vm4(t+dt/2))

    // put everything together with proper weights
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_particleBuffer.bindBase(PARTICLE_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
    m_rkM2Buffer.bindBase( RK_MTWO_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_rkM3Buffer.bindBase( RK_MTHREE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_rkM4Buffer.bindBase( RK_MFOUR_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_rk4Shader.dispatch(m_numParticles,m_wgSize);
}


