/*
 * mpUtils
 * ParticleSpawner.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ParticleSpawner class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_PARTICLESPAWNER_H
#define MPUTILS_PARTICLESPAWNER_H

// includes
//--------------------
#include <Graphics/Graphics.h>
#include <Log/Log.h>
#include "Common.h"
//--------------------

//-------------------------------------------------------------------
/**
 * class ParticleSpawner
 *
 * usage:
 * Construct a object of this class to compile the needed spawning shader.
 * If you have special needs for the particle buffer (eg mappable) use setBufferFlags() to set the Flags you need.
 * Use one of the spawnParticles functions to spawn the particles as a sphere or cube. the particle Buffer will be overwritten
 * in this process. If you want to add particles to an existing buffer make sure to make a copy beforehand. The new Buffer will be bound at
 * the SPAWNER_BUFFER_BINDING specified in the Common.h make sure the same index is used in the shader.
 * Dispatch group size is specified by SPAWNER_GROUP_SIZE.
 * Remember to sync memory after spawning (glMemorieBarrier(...))!
 *
 */
class ParticleSpawner
{
public:
    ParticleSpawner(); //!< constructor will compile shader
    void spawnParticles(uint32_t numParticles, float totalMass, float temp,
                        const glm::vec3& upperBound, const glm::vec3& lowerBound); //!< spawns particles in a cube
    void spawnParticles(uint32_t numParticles, float totalMass, float temp,
                        float radius, const glm::vec3& center={0,0,0} ); //!< spawns particles in a sphere

    // getter
    mpu::gph::Buffer getParticleBuffer()const{return m_particleBuffer;} //!< returns the particle buffer for use by rendering and simulation classes
    uint32_t getNumParticles()const{return m_numParticles;} //!< returns the number of particles that were created
    float getTotalMass()const{return m_totalMass;} //!< returns the total mass of particles created
    float getParticleMass()const{return m_particleMass;} //!< returns the mass of each particle that was created
    float getTotalVolume()const{return m_totalVolume;} //!< returns the total volume the particles represent
    float getParticleVolume()const{return m_particleVolume;} //!< returns the volume each particle represent
    float getParticleDensity()const{return m_particleDensity;} //!< returns the density of each particle

    void setBufferFlags(uint32_t flags){m_bufferFlags = flags;}; //!< set the flags the particle buffer will be created with

private:
    mpu::gph::ShaderProgram m_cubeSpawnShader; //!< shader for spawning particles in a cube
    mpu::gph::ShaderProgram m_sphereSpawnShader; //!< shader for spawning particles in a spere

    uint32_t m_bufferFlags{0}; //!< the flags the particle buffer will be created with
    mpu::gph::Buffer m_particleBuffer; //!< the buffer where the particles are stored

    // info about the last created particles
    uint32_t m_numParticles{0}; //!< the number of particles that are created
    float m_totalMass{0}; //!< the total mass of the created particles
    float m_particleMass{0}; //!< the mass that was assigned to each particle
    float m_totalVolume{0}; //!< volume that is represented by the particles
    float m_particleVolume{0}; //!< volume represented by one particle
    float m_particleDensity{0}; //!< density for each particle
};


#endif //MPUTILS_PARTICLESPAWNER_H
