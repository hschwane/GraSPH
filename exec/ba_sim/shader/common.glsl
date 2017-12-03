#pragma once

// types from the prticle bufffer
#define POS_TYPE vec4
#define VEL_TYPE vec4
#define ACC_TYPE vec4


// buffer bindings
#define RENDERER_POSITION_BUFFER_BINDING 0

#define PARTICLE_BUFFER_BINDING 2
#define PARTICLE_POSITION_BUFFER_BINDING 2
#define PARTICLE_VELOCITY_BUFFER_BINDING 3
#define PARTICLE_ACCELERATION_BUFFER_BINDING 4

// currently invalid
//#define VERLET_BUFFER_BINDING 4
//#define RK_STATE_IN_BUFFER_BINDING 5
//#define RK_DERIV_BUFFER_BINDING 6
//#define RK_OUT_BUFFER_BINDING 7
//#define RK_MTWO_BUFFER_BINDING 8
//#define RK_MTHREE_BUFFER_BINDING 9
//#define RK_MFOUR_BUFFER_BINDING 10

// arrays for rendering data
#define RENDERER_POSITION_ARRAY 0
