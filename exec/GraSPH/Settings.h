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

// units
const long double ly = 9.461e15l; // one light year
const long double pc = 3.086e16l; // one parsec
const long double au = 1.496e11l; // one astronomical unit

const long double Ms = 1.9891e30l; // mass of sun
const long double Mj = 1.8986e27l; // mass of jupiter
const long double Me = 5.9722e24l; // mass of earth

const long double LENGTH_UNIT = 0.25*pc; // the unit of langth in meter (use unit definitions above)
const long double MASS_UNIT = Ms; // the length unit of mass in kg

// time
constexpr double INITIAL_DT = 0.002; // initial timestep
constexpr double MAX_DT = 0.045; // biggest timestep
constexpr double MIN_DT = 0.00001; // smallest timestep
constexpr float GRAV_ACCURACY = 0.05; // the bigger this number the larger timesteps are allowed based on the acceleration criterion
constexpr float COURANT_NUMBER = 0.4; // the bigger this number the larger timesteps are allowed based on the sph criterion

// particles
constexpr float TOTAL_MASS              = 20; // total mass of all particles
constexpr unsigned int NUM_PARTICLES    = 16384; // total number of particles, use power of 2 for convenience
constexpr float SPAWN_RADIUS            = 6; // radius of the initial cloud

// gravity
constexpr float EPS_FACTOR  = 0.2; // a particle behaves like a plummer sphere with a radius equal to its smoothing length multiplied by this factor

// sph
constexpr float A                   = 0.06; // function of specific entropy
constexpr float VISC                = 1; // strength of artificial viscosity
constexpr float BALSARA_STRENGTH    = 0.8; // how much the balsara switch will influence the viscosity

constexpr float AC1      = 1; // adiabatic constant when rho is below FRAG_LIMIT
constexpr float AC2      = 7.0f/5.0f; // adiabatic constant when rho is above FRAG_LIMIT
constexpr float FRAG_LIMIT = 80;

constexpr float NUM_NEIGHBOURS = 50; // the desired number of interaction partners
constexpr float INITIAL_H               = 0.3; // initial kernel radius
constexpr float HMIN    = 0.05; // smallest kernel radius
constexpr float HMAX    = 3; // biggest kernel radius

// visuals
constexpr int HEIGHT = 800; // window size in px
constexpr int WIDTH = 800;
constexpr float PARTICLE_RENDER_SIZE    = 0.008; // radius of a particle
constexpr float PARTICLE_BRIGHTNESS     = 1; // radius of a particle
const glm::vec4 PARTICLE_COLOR          = glm::vec4(0.9,0.3,0.1,1); // color of the particel

// threads and workgroups
constexpr unsigned int DENSITY_THREADS_PER_PARTICLE = 1;
constexpr unsigned int ACCEL_THREADS_PER_PARTICLE   = 1;
constexpr unsigned int DENSITY_WGSIZE               = 256;
constexpr unsigned int PRESSURE_WGSIZE              = 256;

#endif //MPUTILS_SETTINGS_H
