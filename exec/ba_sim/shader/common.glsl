#pragma once

struct Particle
{
    vec4 position;
    vec4 velocity;
    float mass;
    float density;
    float temperature;
    float renderSize;
};

#define PARTICLE_BUFFER_BINDING 2
#define SPAWNER_BUFFER_BINDING 3
