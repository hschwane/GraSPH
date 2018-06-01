/*
 * mpUtils
 * Common.h
 *
 * This header file defines some global constants.
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */
#ifndef MPUTILS_DATATYPES_H_H
#define MPUTILS_DATATYPES_H_H

//-------------------------------------------------------------------
// buffer bindings
constexpr unsigned int RENDERER_POSITION_BUFFER_BINDING = 0;

constexpr unsigned int PARTICLE_BUFFER_BINDING = 2;
constexpr unsigned int PARTICLE_POSITION_BUFFER_BINDING = 2;
constexpr unsigned int PARTICLE_VELOCITY_BUFFER_BINDING = 3;
constexpr unsigned int PARTICLE_ACCELERATION_BUFFER_BINDING = 4;
constexpr unsigned int PARTICLE_HYDRO_BUFFER_BINDING = 5;
constexpr unsigned int PARTICLE_SMLENGTH_BUFFER_BINDING = 6;
constexpr unsigned int PARTICLE_TIMESTEP_BUFFER_BINDING = 7;
constexpr unsigned int PARTICLE_BALSARA_BUFFER_BINDING = 8;

constexpr unsigned int RENDERER_POSITION_ARRAY = 0;
constexpr unsigned int RENDERER_MASS_ARRAY = 1;

// work group size
constexpr unsigned int GENERAL_WGSIZE = 128;

#endif //MPUTILS_DATATYPES_H_H
