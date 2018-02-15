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
 * struct Sphere
 *
 * Defines a sphere to be passed to the particle Spawners spawn methods.
 */
struct Sphere
{
    float radius; // the radius of the sphere
    glm::vec3 center; // the center of the sphere
    float frac; // the fraction of total particles to be placed in this sphere
};

//-------------------------------------------------------------------
/**
 * class ParticleSpawner
 *
 * usage:
 * Construct a object of this class to compile the needed spawning shader.
 * If you have special needs for the particle buffer (eg mappable) use setBufferFlags() to set the Flags you need.
 * Use one of the spawnParticles functions to spawn the particles as a sphere or cube. the particle Buffer will be overwritten
 * in this process. If you want to add particles to an existing buffer make sure to make a copy beforehand. The new Buffer will be bound at
 * the PARTICLE_BUFFER_BINDING specified in the Common.h make sure the same index is used in the shader.
 * Remember to sync memory after spawning (glMemorieBarrier(...))!
 *
 */
class ParticleSpawner
{
public:
    ParticleSpawner(); //!< constructor will compile shader
    void spawnParticlesCube(const float totalMass, const glm::vec3 &upperBound, const glm::vec3 &lowerBound); //!< spawns particles in a cube
    void spawnParticlesSphere(const float totalMass, const float radius, const glm::vec3 &center = {0, 0, 0}); //!< spawns particles in a sphere
    void spawnParticlesMultiSphere(const float totalMass, const std::vector<Sphere> spheres); //!< spawns particles in a multiple spheres

    void addSimplexVelocityField(float frequency, float scale, int seed); //!< adds a initial random velocity field based on simplex noise to the particles
    void addCurlVelocityField(float frequency, float scale, int seed); //!< adds a initial random velocity fiels based on curl noise to the particles
    void addMultiFrequencyCurl(std::vector<std::pair<float, float>> freq, int seed, float hmin, float hmax); //!< add multiple frequencies of simplex noise and calculate the curl using sph methods (him and max are parameters for the adjust-H-shader)
    void addAngularVelocity(glm::vec3 axis); //!< adds a angular velocity around the axis "Axis" speed depends on the length of axis

    // getter
    ParticleBuffer getParticleBuffer()const{return m_particleBuffer;} //!< returns the particle buffer for use by rendering and simulation classes
    float getTotalMass()const{return m_totalMass;} //!< returns the total mass of particles created
    float getParticleMass()const{return m_particleMass;} //!< returns the mass of each particle that was created
    float getTotalVolume()const{return m_totalVolume;} //!< returns the total volume the particles represent
    float getParticleVolume()const{return m_particleVolume;} //!< returns the volume each particle represent
    float getParticleDensity()const{return m_particleDensity;} //!< returns the density of each particle

    void setBuffer(ParticleBuffer buffer){m_particleBuffer=buffer;} //!< set the buffer that is filled with particles

private:
    mpu::gph::ShaderProgram m_cubeSpawnShader; //!< shader for spawning particles in a cube
    mpu::gph::ShaderProgram m_sphereSpawnShader; //!< shader for spawning particles in a spere
    mpu::gph::ShaderProgram m_initialVelocitySimplexShader; //!< shader for adding initial velocity
    mpu::gph::ShaderProgram m_initialVelocityCurlShader; //!< shader for adding initial velocity
    mpu::gph::ShaderProgram m_addSimplexShader; //!< shader for adding simplex noise to the particles acceleration field
    mpu::gph::ShaderProgram m_angVelShader; //!< shader for adding rotational velocity

    ParticleBuffer m_particleBuffer; //!< the buffer where the particles are stored

    // info about the last created particles
    float m_totalMass{0}; //!< the total mass of the created particles
    float m_particleMass{0}; //!< the mass that was assigned to each particle
    float m_totalVolume{0}; //!< volume that is represented by the particles
    float m_particleVolume{0}; //!< volume represented by one particle
    float m_particleDensity{0}; //!< density for each particle
};


#endif //MPUTILS_PARTICLESPAWNER_H
