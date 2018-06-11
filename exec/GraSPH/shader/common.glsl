#pragma once

// feature options
#define BALSARA_SWITCH
#define DH_DENSITY_CORRECTION
#define ARTIFICIAL_HEATING

// handle some dependencys between features
#ifdef DH_DENSITY_CORRECTION
    #define D_RHO_D_H
#endif

// buffer bindings
#define RENDERER_POSITION_BUFFER_BINDING 0

#define PARTICLE_BUFFER_BINDING 2
#define PARTICLE_POSITION_BUFFER_BINDING 2
#define PARTICLE_VELOCITY_BUFFER_BINDING 3
#define PARTICLE_ACCELERATION_BUFFER_BINDING 4
#define PARTICLE_HYDRO_BUFFER_BINDING 5
#define PARTICLE_SMLENGTH_BUFFER_BINDING 6
#define PARTICLE_TIMESTEP_BUFFER_BINDING 7
#define PARTICLE_BALSARA_BUFFER_BINDING 8
#define PARTICLE_STRESS_BUFFER_BINDING 9
#define PARTICLE_DSTRESS_DT_BUFFER_BINDING 9

// arrays for rendering data
#define RENDERER_POSITION_ARRAY 0
#define RENDERER_MASS_ARRAY 1

// defines for buffer access

// position buffer
#define POSITION xyz
#define MASS w

// velocity buffer
#define VELOCITY xyz
#define SPEED_OF_SOUND w

// acceleration buffer
#define ACCEL xyz
#define MAXVSIG w

// hydro buffer
#define DENSITY x
#define PRESSURE y
#define BASWITCH z
#define DH_DENSITY_FACTOR w

// balsara buffer
#define CURL xyz
#define DIV w

