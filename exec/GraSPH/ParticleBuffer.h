/*
 * GraSPH
 * ParticleBuffer.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ParticleBuffer class
 *
 * Copyright (c) 2018 Hendrik Schwanekamp
 *
 */

#ifndef GRASPH_PARTICLEBUFFER_H
#define GRASPH_PARTICLEBUFFER_H

// includes
//--------------------
#include <Graphics/Graphics.h>
//--------------------

//-------------------------------------------------------------------
/**
 * class ParticleBuffer
 *
 * @brief ParticleBuffer contains a set of openGL buffers that contain all the particle attributes
 * size is only updated when reallocate all is used.
 */
class ParticleBuffer
{
public:
    typedef glm::vec4 posType; // w is mass for gravity
    typedef glm::vec4 velType; // w is the speed of sound
    typedef glm::vec4 accType; // w is max(vsig) over all neighbours of the particle (used for timestep criterion)
    typedef glm::vec4 hydrodynamicsType; // x is pressure, y is density, z is the vorticity viscosity correct2ion, w is a factor needed for pressure calc
    typedef float smlengthType;
    typedef float timestepType;
    typedef glm::vec4 balsaraType;

    ParticleBuffer()= default;
    explicit ParticleBuffer(uint32_t numParticles, uint32_t accMulti = 1, uint32_t hydroMulti = 1, bool balsara = true, GLbitfield flags = 0);
    void reallocateAll(uint32_t numParticles, uint32_t accMulti = 1,  uint32_t hydroMulti = 1, bool balsara = true, GLbitfield flags = 0);
    void bindAll( uint32_t binding, GLenum target);


    uint32_t size(){ return m_numberOfParticles;} //!< returns the number of particles
    uint32_t accPerParticle(){ return m_accMulti;} //!< returns the number of different accelerations that can be stored per particle (actually one more acceleration per particle can be stored to allow storing of the acceleration at t-1)
    uint32_t hydPerParticle(){ return m_hydMulti;} //!< returns the number of different hydro states that can be stored per particle
    bool hasBalsara(){ return m_balsara;} //!< returns true if this buffer contains a balsara buffer

    mpu::gph::Buffer positionBuffer;
    mpu::gph::Buffer velocityBuffer;
    mpu::gph::Buffer accelerationBuffer;
    mpu::gph::Buffer hydrodynamicsBuffer;
    mpu::gph::Buffer smlengthBuffer;
    mpu::gph::Buffer timestepBuffer;
    mpu::gph::Buffer balsaraBuffer;
    mpu::gph::Buffer stressBuffer;
    mpu::gph::Buffer edotBuffer;
    mpu::gph::Buffer rdotBuffer;

private:
    uint32_t m_numberOfParticles;
    uint32_t m_accMulti;
    uint32_t m_hydMulti;
    bool m_balsara;
};


#endif //GRASPH_PARTICLEBUFFER_H
