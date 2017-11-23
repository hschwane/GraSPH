/*
 * mpUtils
 * ParticleSpawner.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ParticleSpawner class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "ParticleSpawner.h"
#include <cmath>
//--------------------

// function definitions of the ParticleSpawner class
//-------------------------------------------------------------------
ParticleSpawner::ParticleSpawner()
        : m_cubeSpawnShader({{PROJECT_SHADER_PATH"ParticleSpawner/cubeSpawn.comp"}}),
          m_sphereSpawnShader({{PROJECT_SHADER_PATH"ParticleSpawner/sphereSpawn.comp"}})
{
}

void ParticleSpawner::spawnParticles(const uint32_t numParticles, const float totalMass, const float temp,
                                const glm::vec3& upperBound, const glm::vec3& lowerBound)
{
    logINFO("Spawner") << "Spawning " << numParticles << " in a cube volume from " << glm::to_string(lowerBound)
                       << " to " << glm::to_string(upperBound);

    m_numParticles = numParticles;
    m_totalMass = totalMass;

    // calculate particle attributes
    m_particleMass = m_totalMass / m_numParticles;
    glm::vec3 cubeSize = upperBound - lowerBound;
    m_totalVolume = cubeSize.x * cubeSize.y * cubeSize.z;
    m_particleVolume = m_totalVolume / m_numParticles;
    m_particleDensity = m_particleVolume / m_particleMass;

    logINFO("Spawner") << "Total volume: " << m_totalVolume;

    logINFO("Spawner") << "Particle attributes: mass=" << m_particleMass << "; volume=" << m_particleVolume
                       << " density=" << m_particleDensity << "; temperature=" << temp
                       << "; renderSize=" << PARTICLE_RENDER_SIZE;

    // generate a particle buffer
    m_particleBuffer.recreate();
    m_particleBuffer.allocate<Particle>( m_numParticles, m_bufferFlags);
    m_particleBuffer.bindBase(SPAWNER_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    logDEBUG("Spawner") << "Buffer created and bound at binding " <<  SPAWNER_BUFFER_BINDING;

    // call the shader to do the work
    m_cubeSpawnShader.uniform3f("upperBound",upperBound);
    m_cubeSpawnShader.uniform3f("lowerBound",lowerBound);
    m_cubeSpawnShader.uniform1f("mass",m_particleMass);
    m_cubeSpawnShader.uniform1f("density",m_particleDensity);
    m_cubeSpawnShader.uniform1f("temperature",temp);
    m_cubeSpawnShader.uniform1f("renderSize",PARTICLE_RENDER_SIZE);
    m_cubeSpawnShader.uniform1f("numOfParticles",m_numParticles);
    m_cubeSpawnShader.uniform1ui("randomSeed", std::time(nullptr)); // time as pseudo random seed
    m_cubeSpawnShader.dispatch(numParticles,SPAWNER_GROUP_SIZE);
}

void ParticleSpawner::spawnParticles(const uint32_t numParticles, const float totalMass, const float temp,
                                     const float radius, const glm::vec3& center)
{
    logINFO("Spawner") << "Spawning " << numParticles << " in a sphere volume at position " << glm::to_string(center)
                       << " with radius " << radius;

    m_numParticles = numParticles;
    m_totalMass = totalMass;

    // calculate particle attributes
    m_particleMass = m_totalMass / m_numParticles;
    m_totalVolume = 3.0/3.0 * M_PI * std::pow(radius,3);
    m_particleVolume = m_totalVolume / m_numParticles;
    m_particleDensity = m_particleVolume / m_particleMass;

    logINFO("Spawner") << "Total volume: " << m_totalVolume;

    logINFO("Spawner") << "Particle attributes: mass=" << m_particleMass << "; volume=" << m_particleVolume
                       << "; density=" << m_particleDensity << "; temperature=" << temp
                       << "; renderSize=" << PARTICLE_RENDER_SIZE;

    // generate a particle buffer
    m_particleBuffer.recreate();
    m_particleBuffer.allocate<Particle>( m_numParticles, m_bufferFlags);
    m_particleBuffer.bindBase(SPAWNER_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    logDEBUG("Spawner") << "Buffer created and bound at binding " <<  SPAWNER_BUFFER_BINDING;

    // call the shader to do the work
    m_sphereSpawnShader.uniform3f("center",center);
    m_sphereSpawnShader.uniform1f("radius",radius);
    m_sphereSpawnShader.uniform1f("mass",m_particleMass);
    m_sphereSpawnShader.uniform1f("density",m_particleDensity);
    m_sphereSpawnShader.uniform1f("temperature",temp);
    m_sphereSpawnShader.uniform1f("renderSize",PARTICLE_RENDER_SIZE);
    m_sphereSpawnShader.uniform1f("numOfParticles",m_numParticles);
    m_sphereSpawnShader.uniform1ui("randomSeed", std::time(nullptr)); // time as pseudo random seed
    m_sphereSpawnShader.dispatch(numParticles,SPAWNER_GROUP_SIZE);

    logDEBUG("Spawner") << "Generation";
}
