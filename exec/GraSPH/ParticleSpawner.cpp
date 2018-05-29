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
          m_initialVelocitySimplexShader({{PROJECT_SHADER_PATH"ParticleSpawner/initialVelocitySimplex.comp"}}),
          m_initialVelocityCurlShader({{PROJECT_SHADER_PATH"ParticleSpawner/initialVelocityCurl.comp"}}),
          m_addSimplexShader({{PROJECT_SHADER_PATH"ParticleSpawner/addPotential.comp"}}),
          m_angVelShader({{PROJECT_SHADER_PATH"ParticleSpawner/angVel.comp"}}),
          m_sphereSpawnShader({{PROJECT_SHADER_PATH"ParticleSpawner/sphereSpawn.comp"}})
{
}

void ParticleSpawner::spawnParticlesCube(const float totalMass, const glm::vec3 &upperBound, const glm::vec3 &lowerBound, const float initialSmlength)
{
    logINFO("Spawner") << "Spawning " << m_particleBuffer.size() << " in a cube volume from " << glm::to_string(lowerBound)
                       << " to " << glm::to_string(upperBound);

    // generate a particle buffer
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    logDEBUG("Spawner") << "Buffer bound at PARTICLE_BUFFER_BINDING.";

    m_totalMass = totalMass;

    // calculate particle attributes
    m_particleMass = m_totalMass / m_particleBuffer.size();
    glm::vec3 cubeSize = upperBound - lowerBound;
    m_totalVolume = cubeSize.x * cubeSize.y * cubeSize.z;
    m_particleVolume = m_totalVolume / m_particleBuffer.size();
    m_particleDensity = m_particleMass / m_particleVolume;

    logINFO("Spawner") << "Total volume: " << m_totalVolume;

    logINFO("Spawner") << "Particle attributes: mass=" << m_particleMass << "; volume=" << m_particleVolume
                       << " density=" << m_particleDensity;

    // call the shader to do the work
    m_cubeSpawnShader.uniform3f("upper_bound",upperBound);
    m_cubeSpawnShader.uniform3f("lower_bound",lowerBound);
    m_cubeSpawnShader.uniform1f("mass",m_particleMass);
    m_cubeSpawnShader.uniform1f("initial_smlength", initialSmlength);
    m_cubeSpawnShader.uniform1ui("random_seed", std::time(nullptr)); // time as pseudo random seed
    m_cubeSpawnShader.uniform1ui("num_of_particles", m_particleBuffer.size());
    m_cubeSpawnShader.uniform1ui("accMulti", m_particleBuffer.accPerParticle());
    m_cubeSpawnShader.uniform1ui("hydMulti", m_particleBuffer.hydPerParticle());
    m_cubeSpawnShader.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));
}

void ParticleSpawner::spawnParticlesSphere(const float totalMass, const float radius, const float initialSmlength, const glm::vec3 &center)
{
    logINFO("Spawner") << "Spawning " << m_particleBuffer.size() << " in a sphere volume at position " << glm::to_string(center)
                       << " with radius " << radius;

    // generate a particle buffer
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    logDEBUG("Spawner") << "Buffer created and bound at PARTICLE_BUFFER_BINDING.";

    m_totalMass = totalMass;

    // calculate particle attributes
    m_particleMass = m_totalMass / m_particleBuffer.size();
    m_totalVolume = 3.0/3.0 * M_PI * std::pow(radius,3);
    m_particleVolume = m_totalVolume / m_particleBuffer.size();
    m_particleDensity = m_particleMass / m_particleVolume;

    logINFO("Spawner") << "Total volume: " << m_totalVolume;

    logINFO("Spawner") << "Particle attributes: mass=" << m_particleMass << "; volume=" << m_particleVolume
                       << "; density=" << m_particleDensity;

    // call the shader to do the work
    m_sphereSpawnShader.uniform3f("center",center);
    m_sphereSpawnShader.uniform1f("radius",radius);
    m_sphereSpawnShader.uniform1f("mass",m_particleMass);
    m_sphereSpawnShader.uniform1f("initial_smlength", initialSmlength);
    m_sphereSpawnShader.uniform1ui("random_seed", std::time(nullptr)); // time as pseudo random seed
    m_sphereSpawnShader.uniform1ui("num_of_particles", m_particleBuffer.size());
    m_sphereSpawnShader.uniform1ui("accMulti", m_particleBuffer.accPerParticle());
    m_sphereSpawnShader.uniform1ui("hydMulti", m_particleBuffer.hydPerParticle());
    m_sphereSpawnShader.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));
}

void ParticleSpawner::spawnParticlesMultiSphere(const float totalMass, const std::vector<Sphere> spheres, const float initialSmlength)
{
    logINFO("Spawner") << "Spawning " << m_particleBuffer.size() << " in " << spheres.size() << " Spheres.";

    // bind the particle buffer
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);

    logDEBUG("Spawner") << "Buffer bound at PARTICLE_BUFFER_BINDING.";

    m_totalMass = totalMass;

    // calculate particle attributes
    m_particleMass = m_totalMass / m_particleBuffer.size();

    logINFO("Spawner") << "Particle attributes: mass=" << m_particleMass;

    uint32_t writtenParticles = 0;
    // spawn all the spheres
    for(auto &&s : spheres)
    {
        uint32_t particles = static_cast<uint32_t>(m_particleBuffer.size() * s.frac);
        logINFO("Spawner") << "Spawning " << particles << " particles in a sphere with radius=" << s.radius << " at "
                           << glm::to_string(s.center);
        m_sphereSpawnShader.uniform3f("center", s.center);
        m_sphereSpawnShader.uniform1f("radius", s.radius);
        m_sphereSpawnShader.uniform1f("mass", m_particleMass);
        m_sphereSpawnShader.uniform1f("initial_smlength", initialSmlength);
        m_sphereSpawnShader.uniform1ui("random_seed", std::time(nullptr)); // time as pseudo random seed
        m_sphereSpawnShader.uniform1ui("num_of_particles", particles);
        m_sphereSpawnShader.uniform1ui("particle_offset", writtenParticles);
        m_sphereSpawnShader.uniform1ui("accMulti", m_particleBuffer.accPerParticle());
        m_sphereSpawnShader.uniform1ui("hydMulti", m_particleBuffer.hydPerParticle());
        m_sphereSpawnShader.dispatch(particles, calcWorkgroupSize(particles));
        writtenParticles += particles;
    }

    logINFO("Spawner") << "Spawned a total of " << writtenParticles << " particles.";
    if(writtenParticles != m_particleBuffer.size())
    {
        logWARNING("Spawner") << "Sphere ratios do not sum up to 1. Particles spawned: " << writtenParticles
                              << " desired amount: " << m_particleBuffer.size();
        m_sphereSpawnShader.uniform1ui("particle_offset", writtenParticles);
        m_sphereSpawnShader.dispatch(m_particleBuffer.size()-writtenParticles, calcWorkgroupSize(m_particleBuffer.size()-writtenParticles));
    }
}

void ParticleSpawner::addSimplexVelocityField(float frequency, float scale, int seed)
{
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_initialVelocitySimplexShader.uniform1i("seed", seed);
    m_initialVelocitySimplexShader.uniform1f("frequency", frequency);
    m_initialVelocitySimplexShader.uniform1f("scale", scale);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_initialVelocitySimplexShader.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));
}

void ParticleSpawner::addCurlVelocityField(float frequency, float scale, int seed)
{
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_initialVelocityCurlShader.uniform1i("seed", seed);
    m_initialVelocityCurlShader.uniform1f("frequency", frequency);
    m_initialVelocityCurlShader.uniform1f("scale", scale);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_initialVelocityCurlShader.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));
}

void ParticleSpawner::addMultiFrequencyCurl(std::vector<std::pair<float, float>> freq, int seed, float hmin, float hmax, float massPerParticle)
{
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    mpu::gph::ShaderProgram doCurlShader({{PROJECT_SHADER_PATH"ParticleSpawner/doSPHCurl.comp"}},
                                         {
                                                 {"WGSIZE",{mpu::toString(calcWorkgroupSize(m_particleBuffer.size()))}},
                                                 {"NUM_PARTICLES",{mpu::toString(m_particleBuffer.size())}},
                                                 {"TILES_PER_THREAD",{mpu::toString(m_particleBuffer.size() / calcWorkgroupSize(m_particleBuffer.size()) / 1)}}
                                         });

    mpu::gph::ShaderProgram densityShader({{PROJECT_SHADER_PATH"Simulation/calculateDensity.comp"}},
                                          {
                                                  {"WGSIZE",{mpu::toString(calcWorkgroupSize(m_particleBuffer.size()))}},
                                                  {"NUM_PARTICLES",{mpu::toString(m_particleBuffer.size())}},
                                                  {"TILES_PER_THREAD",{mpu::toString(m_particleBuffer.size() / calcWorkgroupSize(m_particleBuffer.size()) / 1)}}
                                          });

    mpu::gph::ShaderProgram adjustH({{PROJECT_SHADER_PATH"Simulation/calculateH.comp"}});
    adjustH.uniform1f("hmin", hmin);
    adjustH.uniform1f("hmax", hmax);
    adjustH.uniform1f("mass_per_particle", massPerParticle);
    adjustH.uniform1f("num_neighbours",50);

    // generate the potential field
    srand(seed);
    auto randSeed = []{return (rand() % static_cast<int>(9999));};
    for(auto &&item : freq)
    {
        m_addSimplexShader.uniform1i("seed", randSeed());
        m_addSimplexShader.uniform1f("frequency", item.first);
        m_addSimplexShader.uniform1f("scale", item.second);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        m_addSimplexShader.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));
    }

    // adjust the smoothing length to something usefull
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    densityShader.dispatch(m_particleBuffer.size()*1/calcWorkgroupSize(m_particleBuffer.size()));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    adjustH.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));

    // another iteration
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    densityShader.dispatch(m_particleBuffer.size()*1/calcWorkgroupSize(m_particleBuffer.size()));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    adjustH.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));

    // now calculate the curl
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    densityShader.dispatch(m_particleBuffer.size()*1/calcWorkgroupSize(m_particleBuffer.size()));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    doCurlShader.dispatch(m_particleBuffer.size()*1/calcWorkgroupSize(m_particleBuffer.size()));
}

void ParticleSpawner::addAngularVelocity(glm::vec3 axis)
{
    m_particleBuffer.bindAll(PARTICLE_BUFFER_BINDING, GL_SHADER_STORAGE_BUFFER);
    m_angVelShader.uniform3f("axis", axis);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_angVelShader.dispatch(m_particleBuffer.size(),calcWorkgroupSize(m_particleBuffer.size()));
}
