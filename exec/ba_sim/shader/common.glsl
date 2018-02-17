#pragma once

// types from the prticle bufffer
#define POS_TYPE vec4
#define VEL_TYPE vec4
#define ACC_TYPE vec4
#define HYDRO_TYPE vec4
#define SMLENGTH_TYPE float
#define TIMESTEP_TYPE vec2
#define ACTIVE_TYPE int


// buffer bindings
#define RENDERER_POSITION_BUFFER_BINDING 0

#define PARTICLE_BUFFER_BINDING 2
#define PARTICLE_POSITION_BUFFER_BINDING 2
#define PARTICLE_VELOCITY_BUFFER_BINDING 3
#define PARTICLE_ACCELERATION_BUFFER_BINDING 4
#define PARTICLE_HYDRO_BUFFER_BINDING 5
#define PARTICLE_SMLENGTH_BUFFER_BINDING 6
#define PARTICLE_TIMESTEP_BUFFER_BINDING 7
#define PARTICLE_ACTIVE_BUFFER_BINDING 8


// arrays for rendering data
#define RENDERER_POSITION_ARRAY 0
#define RENDERER_MASS_ARRAY 1
