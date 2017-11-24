/*
 * mpUtils
 * Common.h
 *
 * This header file defines some global datatypes and constants.
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */
#ifndef MPUTILS_DATATYPES_H_H
#define MPUTILS_DATATYPES_H_H

// includes
//--------------------
#include <Graphics/Graphics.h>
#include <Log/Log.h>
//--------------------


//-------------------------------------------------------------------
/**
 * @brief Particle struct representing one particle
 */
struct Particle
{
    glm::vec4 position{0,0,0,1};
    glm::vec4 velocity{0};
    float mass{0};
    float density{0};
    float temperature{0};
    float renderSize{0};
};

//-------------------------------------------------------------------
// global variables for settings TODO: move to init file / gui eventually

// gl
constexpr unsigned int PARTICLE_BUFFER_BINDING = 2;
constexpr unsigned int SPAWNER_BUFFER_BINDING = 3;
constexpr unsigned int SPAWNER_GROUP_SIZE = 500;
constexpr unsigned int RENDERER_BUFFER_BINDING = 1;
constexpr unsigned int RENDERER_POSITION_ARRAY = 0;
constexpr unsigned int RENDERER_SIZE_ARRAY = 1;


// simulation
constexpr double DT = 0.001;
constexpr double EPS2 = 0.0001;
constexpr float G = 1;

// particles
constexpr float PARTICLE_RENDER_SIZE = 0.008; // radius of a particle
constexpr float TEMPERATURE = 30;

// spawning
constexpr float TOTAL_MASS = 1;
constexpr unsigned int NUM_PARTICLES = 300;
const  glm::vec3 LOWER_BOUND = glm::vec3(-1,-1,-1);
const  glm::vec3 UPPER_BOUND = glm::vec3(1,1,1);

#endif //MPUTILS_DATATYPES_H_H
