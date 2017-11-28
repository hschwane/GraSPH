/*
 * mpUtils
 * Leapfrog.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Leapfrog class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "Leapfrog.h"
//--------------------

// namespace
//--------------------

//--------------------

// function definitions of the Leapfrog class
//-------------------------------------------------------------------
Leapfrog::Leapfrog() : Leapfrog(nullptr,mpu::gph::Buffer(),0,0)
{

}

Leapfrog::Leapfrog(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number,
                   double dt) : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC),
                                m_shader({{PROJECT_SHADER_PATH"DEsolver/leapfrog.comp"}}),
                                m_calcAcceleration(accelerator),
                                m_numParticles(number), m_wgSize(calcWorkgroupSize(number)),
                                m_dt(dt)
{
    m_shader.uniform1f("dt",dt);
    m_shader.uniform1f("vel_dt",dt/2);
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
}

void Leapfrog::setAccelerator(std::function<void(void)> accelerator)
{
    m_calcAcceleration = accelerator;
}

void Leapfrog::setDT(double dt)
{
    m_dt = dt;
    m_shader.uniform1f("dt",dt);
    m_shader.uniform1f("vel_dt",dt);
}

void Leapfrog::setParticles(mpu::gph::Buffer particleBuffer, uint32_t number)
{
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = number;
    m_wgSize = calcWorkgroupSize(number);
}

void Leapfrog::start()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
    m_shader.dispatch(m_numParticles,m_wgSize);
    m_shader.uniform1f("vel_dt",m_dt);
}

void Leapfrog::advanceTime()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
    m_shader.dispatch(m_numParticles,m_wgSize);
}

void Leapfrog::catchUpTime()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
    m_shader.uniform1f("vel_dt",m_dt/2);
    m_shader.uniform1f("dt",0);
    m_shader.dispatch(m_numParticles,m_wgSize);
    m_shader.uniform1f("dt",m_dt);
}
