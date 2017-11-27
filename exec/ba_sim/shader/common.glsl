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

#define VERLET_BUFFER_BINDING 4
#define RK_STATE_IN_BUFFER_BINDING 5
#define RK_DERIV_BUFFER_BINDING 6
#define RK_OUT_BUFFER_BINDING 7
#define RK_MTWO_BUFFER_BINDING 8
#define RK_MTHREE_BUFFER_BINDING 9
#define RK_MFOUR_BUFFER_BINDING 10

// arrays for rendering data
#define RENDERER_POSITION_ARRAY 0
#define RENDERER_SIZE_ARRAY 1
