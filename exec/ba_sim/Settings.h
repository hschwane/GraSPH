/*
 * mpUtils
 * Settings.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2018 Hendrik Schwanekamp
 *
 */
#ifndef MPUTILS_SETTINGS_H
#define MPUTILS_SETTINGS_H

// includes
//--------------------
#include <Graphics/Graphics.h>
//--------------------

// time
constexpr double DT = 0.003;

// particles
constexpr float TOTAL_MASS              = 20;//1e-22*1.98892e30 /1000;
constexpr unsigned int NUM_PARTICLES    = 16000;
constexpr float SPAWN_RADIUS            = 6;
const glm::vec3 LOWER_BOUND             = glm::vec3(-1,-1,-1);
const glm::vec3 UPPER_BOUND             = glm::vec3(1,1,1);

// gravity
constexpr double EPS2       = 0.02;
constexpr double EPS2_SPH   = 0.002;
constexpr float G           = 1;//6.67408e-11 ;//* 1e-9 *1000;

// sph
constexpr float K       = 0.04;
constexpr float VISC    = 0.02;
constexpr float HMIN    = 0.02;
constexpr float HMAX    = 3;
constexpr float HEPS    = 0.005;
constexpr float AC      = 1;

const float SINK_R  = 0.4;
const float SINK_TH = 4;

// visuals
constexpr float PARTICLE_RENDER_SIZE    = 0.008; // radius of a particle
constexpr float PARTICLE_BRIGHTNESS     = 1; // radius of a particle
const glm::vec4 PARTICLE_COLOR          = glm::vec4(0.9,0.3,0.1,1);

// threads and workgroups
constexpr unsigned int DENSITY_THREADS_PER_PARTICLE = 4;
constexpr unsigned int ACCEL_THREADS_PER_PARTICLE   = 2;
constexpr unsigned int DENSITY_WGSIZE               = 128;
constexpr unsigned int PRESSURE_WGSIZE              = 128;

#endif //MPUTILS_SETTINGS_H
