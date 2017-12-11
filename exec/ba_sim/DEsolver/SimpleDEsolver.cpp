/*
 * mpUtils
 * SimpleDEsolver.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "SimpleDEsolver.h"
//--------------------

// function definitions of the SimpleDEsolver class
//-------------------------------------------------------------------
SimpleDEsolver::SimpleDEsolver(std::string shaderPath) : SimpleDEsolver(shaderPath,nullptr,ParticleBuffer(),0)
{
}

SimpleDEsolver::SimpleDEsolver(std::string shaderPath,std::function<void(void)> accelerator, ParticleBuffer particleBuffer, double dt)
                                          : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC | DEsolverFlag::IS_SELF_STARTING),
                                            m_shader({{shaderPath}}),
                                            m_calcAcceleration(accelerator),
                                            m_numParticles(particleBuffer.size()),
                                            m_wgSize(calcWorkgroupSize(particleBuffer.size()))

{
    m_shader.uniform1f("dt",dt);
    particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
}

void SimpleDEsolver::setAccelerator(std::function<void(void)> accelerator)
{
    m_calcAcceleration = accelerator;
}

void SimpleDEsolver::setDT(double dt)
{
    m_shader.uniform1f("dt",dt);
}

void SimpleDEsolver::setParticles(ParticleBuffer particleBuffer)
{
    particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BARRIER_BIT);
    m_numParticles = particleBuffer.size();
    m_wgSize = calcWorkgroupSize(particleBuffer.size());
}

void SimpleDEsolver::advanceTime()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_shader.dispatch(m_numParticles,m_wgSize);
}

