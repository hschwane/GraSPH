#pragma once

struct Particle
{
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    float mass;
    float density;
    float temperature;
    float renderSize;
};

// buffer for spawning
#define SPAWNER_BUFFER_BINDING 3

// buffer for simulation
#define PARTICLE_BUFFER_BINDING 2
#define VERLET_LAST_POS_BUFFER_BINDING 4

// arrays for rendering data
#define RENDERER_POSITION_ARRAY 0
#define RENDERER_SIZE_ARRAY 1
