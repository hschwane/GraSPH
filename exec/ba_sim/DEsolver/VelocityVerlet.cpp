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
VelocityVerlet::VelocityVerlet() : VelocityVerlet(nullptr,ParticleBuffer(),0)
{

}

VelocityVerlet::VelocityVerlet(std::function<void(void)> accelerator, ParticleBuffer particleBuffer, double dt)
                      : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC),
                        m_shader({{PROJECT_SHADER_PATH"DEsolver/velocityVerlet.comp"}}),
                        m_calcAcceleration(accelerator), m_dt(dt),
                        m_numParticles(particleBuffer.size()),
                        m_wgSize(calcWorkgroupSize(particleBuffer.size()))
{
    m_shader.uniform1f("dt",dt);
    m_shader.uniform1f("vel_dt",dt);
    particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
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

void VelocityVerlet::setParticles(ParticleBuffer particleBuffer)
{
    particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = particleBuffer.size();
    m_wgSize = calcWorkgroupSize(particleBuffer.size());
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

