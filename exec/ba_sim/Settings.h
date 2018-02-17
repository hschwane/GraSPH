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
constexpr double DT = 0.002;

// particles
constexpr float TOTAL_MASS              = 20;//1e-22*1.98892e30 /1000;
constexpr unsigned int NUM_PARTICLES    = 16000;
constexpr float SPAWN_RADIUS            = 6;
const glm::vec3 LOWER_BOUND             = glm::vec3(-1,-1,-1);
const glm::vec3 UPPER_BOUND             = glm::vec3(1,1,1);

// gravity
constexpr double EPS_FACTOR  = 0.2; // a particle behaves like a plummer sphere with a radius equal to its smoothing length multiplied by this factor
constexpr float G           = 1;//6.67408e-11 ;//* 1e-9 *1000;

// sph
constexpr float K       = 0.06;
constexpr float VISC    = 1;
constexpr float HMIN    = 0.05;
constexpr float HMAX    = 3;
constexpr float HEPS_FACTOR    = 0.01;
constexpr float AC1      = 1;
constexpr float AC2      = 7.0f/5.0f;  //13.0f/10.0f; //7.0f/5.0f;
constexpr float FRAG_LIMIT = 30;

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
