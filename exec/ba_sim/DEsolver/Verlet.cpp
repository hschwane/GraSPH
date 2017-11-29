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
Verlet::Verlet() : Verlet(nullptr,mpu::gph::Buffer(),0,0)
{

}

Verlet::Verlet(std::function<void(void)> accelerator, mpu::gph::Buffer particleBuffer, uint32_t number
        , double dt)  : DEsolver(DEsolverFlag::REQUIRE_ONE_ACC),
                        m_shader({{PROJECT_SHADER_PATH"DEsolver/verlet.comp"}}),
                        m_startShader({{PROJECT_SHADER_PATH"DEsolver/verletFirstStep.comp"}}),
                        m_calcAcceleration(accelerator),
                        m_numParticles(number), m_wgSize(calcWorkgroupSize(number))
{
    m_shader.uniform1f("dt",dt);
    m_startShader.uniform1f("dt",dt);
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
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

void Verlet::setParticles(mpu::gph::Buffer particleBuffer, uint32_t number)
{
    particleBuffer.bindBase(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_numParticles = number;
    m_wgSize = calcWorkgroupSize(number);
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
