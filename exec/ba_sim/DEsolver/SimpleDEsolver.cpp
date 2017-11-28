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
SimpleDEsolver::SimpleDEsolver(std::string shaderPath) : SimpleDEsolver(shaderPath,nullptr,mpu::gph::Buffer(),0,0)
{
}

SimpleDEsolver::SimpleDEsolver(std::string shaderPath,std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number,
                               double dt) : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC | DEsolverFlag::IS_SELF_STARTING),
                                            m_shader({{shaderPath}}),
                                            m_calcAcceleration(accelerator),
                                            m_numParticles(number), m_wgSize(calcWorkgroupSize(number))

{
    m_shader.uniform1f("dt",dt);
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
}

void SimpleDEsolver::setAccelerator(std::function<void(void)> accelerator)
{
    m_calcAcceleration = accelerator;
}

void SimpleDEsolver::setDT(double dt)
{
    m_shader.uniform1f("dt",dt);
}

void SimpleDEsolver::setParticles(mpu::gph::Buffer particleBuffer, uint32_t number)
{
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = number;
    m_wgSize = calcWorkgroupSize(number);
}

void SimpleDEsolver::advanceTime()
{
    m_calcAcceleration();
    glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
    m_shader.dispatch(m_numParticles,m_wgSize);
}

