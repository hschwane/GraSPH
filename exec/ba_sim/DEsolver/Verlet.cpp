/*
 * mpUtils
 * Verlet.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Verlet class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "Verlet.h"
//--------------------

// namespace
//--------------------

//--------------------

// function definitions of the Verlet class
//-------------------------------------------------------------------
Verlet::Verlet() : Verlet(nullptr,ParticleBuffer(),0)
{

}

Verlet::Verlet(std::function<void(void)> accelerator, ParticleBuffer particleBuffer, double dt)
                        : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC),
                        m_shader({{PROJECT_SHADER_PATH"DEsolver/verlet.comp"}}),
                        m_startShader({{PROJECT_SHADER_PATH"DEsolver/verletFirstStep.comp"}}),
                        m_calcAcceleration(accelerator),
                        m_numParticles(particleBuffer.size()),
                        m_wgSize(calcWorkgroupSize(particleBuffer.size()))
{
    m_shader.uniform1f("dt",dt);
    m_startShader.uniform1f("dt",dt);
    particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    verletBuffer.allocate<glm::vec4>(m_numParticles);
    verletBuffer.bindBase(VERLET_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
}

void Verlet::setAccelerator(std::function<void(void)> accelerator)
{
    m_calcAcceleration = accelerator;
}

void Verlet::setDT(double dt)
{
    m_shader.uniform1f("dt",dt);
    m_startShader.uniform1f("dt",dt);
}

void Verlet::setParticles(ParticleBuffer particleBuffer)
{
    particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = particleBuffer.size();
    m_wgSize = calcWorkgroupSize(particleBuffer.size());
    verletBuffer.recreate();
    verletBuffer.allocate<glm::vec4>(m_numParticles);
    verletBuffer.bindBase(VERLET_BUFFER_BINDING,GL_SHADER_STORAGE_BUFFER);
}

void Verlet::start()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_startShader.dispatch(m_numParticles,m_wgSize);
}

void Verlet::advanceTime()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_shader.dispatch(m_numParticles,m_wgSize);
}
