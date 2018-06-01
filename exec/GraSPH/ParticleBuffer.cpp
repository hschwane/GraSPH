/*
 * GraSPH
 * ParticleBuffer.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ParticleBuffer class
 *
 * Copyright (c) 2018 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "ParticleBuffer.h"
//--------------------

// function definitions of the ParticleBuffer class
//-------------------------------------------------------------------
ParticleBuffer::ParticleBuffer(uint32_t numParticles, uint32_t accMulti, uint32_t hydroMulti, bool balsara,
                               GLbitfield flags)
{
    reallocateAll(numParticles, accMulti, hydroMulti, balsara, flags);
}

void ParticleBuffer::reallocateAll(uint32_t numParticles, uint32_t accMulti, uint32_t hydroMulti, bool balsara,
                                   GLbitfield flags)
{
    positionBuffer.recreate();
    positionBuffer.allocate<posType>(numParticles,flags);
    velocityBuffer.recreate();
    velocityBuffer.allocate<velType>(numParticles,flags);
    accelerationBuffer.recreate();
    accelerationBuffer.allocate<accType>(numParticles*accMulti,flags);
    hydrodynamicsBuffer.recreate();
    hydrodynamicsBuffer.allocate<hydrodynamicsType>(numParticles*hydroMulti,flags);
    smlengthBuffer.recreate();
    smlengthBuffer.allocate<smlengthType>(numParticles,flags);
    timestepBuffer.recreate();
    timestepBuffer.allocate<timestepType>(numParticles,flags);

    if(balsara)
    {
        balsaraBuffer.recreate();
        balsaraBuffer.allocate<balsaraType>(numParticles,flags);
    }

    m_numberOfParticles=numParticles;
    m_accMulti = accMulti;
    m_hydMulti = hydroMulti;
    m_balsara = balsara;
}

void ParticleBuffer::bindAll(uint32_t binding, GLenum target)
{
    positionBuffer.bindBase(binding,target);
    velocityBuffer.bindBase(binding+1,target);
    accelerationBuffer.bindBase(binding+2,target);
    hydrodynamicsBuffer.bindBase(binding+3,target);
    smlengthBuffer.bindBase(binding+4,target);
    timestepBuffer.bindBase(binding+5,target);
    balsaraBuffer.bindBase(binding+6,target);
}