/*
 * mpUtils
 * VelocityVerlet.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the VelocityVerlet class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "VelocityVerlet.h"
//--------------------

// function definitions of the VelocityVerlet class
//-------------------------------------------------------------------
VelocityVerlet::VelocityVerlet() : VelocityVerlet(nullptr,mpu::gph::Buffer(),0,0)
{

}

VelocityVerlet::VelocityVerlet(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number
        , double dt)  : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC),
                        m_shader({{PROJECT_SHADER_PATH"DEsolver/velocityVerlet.comp"}}),
                        m_calcAcceleration(accelerator), m_dt(dt),
                        m_numParticles(number), m_wgSize(calcWorkgroupSize(number))
{
    m_shader.uniform1f("dt",dt);
    m_shader.uniform1f("vel_dt",dt);
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    verletBuffer.allocate<glm::vec4>(m_numParticles);
    verletBuffer.bindBase(VERLET_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
}

void VelocityVerlet::setAccelerator(std::function<void(void)> accelerator)
{
    m_calcAcceleration = accelerator;
}

void VelocityVerlet::setDT(double dt)
{
    m_dt = dt;
    m_shader.uniform1f("dt",dt);
    m_shader.uniform1f("vel_dt",dt);
}

void VelocityVerlet::setParticles(mpu::gph::Buffer particleBuffer, uint32_t number)
{
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = number;
    m_wgSize = calcWorkgroupSize(number);
    verletBuffer.recreate();
    verletBuffer.allocate<glm::vec4>(m_numParticles);
    verletBuffer.bindBase(VERLET_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
}

void VelocityVerlet::start()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_shader.uniform1f("vel_dt",0.0f);
    m_shader.dispatch(m_numParticles,m_wgSize);
    m_shader.uniform1f("vel_dt",m_dt);
}

void VelocityVerlet::advanceTime()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_shader.dispatch(m_numParticles,m_wgSize);
}

