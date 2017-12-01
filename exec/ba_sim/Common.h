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
    glm::vec4 acceleration{0};
    float mass{0};
    float density{0};
    float temperature{0};
    float renderSize{0};
};

//-------------------------------------------------------------------
// global variables for settings TODO: move to init file / gui eventually

// gl
constexpr unsigned int SPAWNER_GROUP_SIZE = 200;
constexpr unsigned int RENDERER_BUFFER_BINDING = 1;

constexpr unsigned int SPAWNER_BUFFER_BINDING = 3;
constexpr unsigned int PARTICLE_BUFFER_BINDING = 2;

constexpr unsigned int VERLET_BUFFER_BINDING = 4;

constexpr unsigned int RK_STATE_IN_BUFFER_BINDING = 5;
constexpr unsigned int RK_DERIV_BUFFER_BINDING = 6;
constexpr unsigned int RK_OUT_BUFFER_BINDING = 7;
constexpr unsigned int RK_MTWO_BUFFER_BINDING = 8;
constexpr unsigned int RK_MTHREE_BUFFER_BINDING = 9;
constexpr unsigned int RK_MFOUR_BUFFER_BINDING = 10;

constexpr unsigned int RENDERER_POSITION_ARRAY = 0;
constexpr unsigned int RENDERER_SIZE_ARRAY = 1;

// simulation
constexpr double DT = 0.032;
constexpr double EPS2 = 0.01;
constexpr float G = 1;

// particles
constexpr float PARTICLE_RENDER_SIZE = 0.008; // radius of a particle
constexpr float TEMPERATURE = 30;

// spawning
constexpr float TOTAL_MASS = 0.005;
constexpr unsigned int NUM_PARTICLES = 12800;
const  glm::vec3 LOWER_BOUND = glm::vec3(-1,-1,-1);
const  glm::vec3 UPPER_BOUND = glm::vec3(1,1,1);

// work group size
constexpr unsigned int FORCED_SIZE = 0;
constexpr unsigned int THREADS_PER_CORE = 32;
constexpr unsigned int THREAD_GROUPING_SIZE = 128;
constexpr unsigned int COMPUTE_CORES = 640;

//-------------------------------------------------------------------
// some help functions

/**
 * @brief calculates a workgroup size that fits the global invocation count based on some assumption on the gpu
 * structure (THREADS_PER_CORE / THREAD_GOUP_SIZE / COMPUTE_CORES) Will probably waste a lot of potential gpu power
 * if totalInvocations % THREADS_PER_CORE != 0 and you have other dispatch calls that could be executed in parallel.
 *
 */
inline uint32_t calcWorkgroupSize(uint32_t totalInvocations)
{
    if(FORCED_SIZE!=0)
        return FORCED_SIZE;

    if(totalInvocations % THREAD_GROUPING_SIZE == 0)
    {
        logINFO("WGSize") << "Selecting workgroup size "<<THREAD_GROUPING_SIZE<<" for " << totalInvocations
                          << " total invocations. Resulting in " << totalInvocations / THREAD_GROUPING_SIZE << " groups";
        return THREAD_GROUPING_SIZE;
    }
    else if (totalInvocations % THREADS_PER_CORE == 0)
    {
        logINFO("WGSize") << "Selecting workgroup size "<<THREADS_PER_CORE<<" for " << totalInvocations
                          << " total invocations. Resulting in " << totalInvocations / THREADS_PER_CORE << " groups";
        return THREADS_PER_CORE;
    }
    else if(totalInvocations <= 32)
    {
        logINFO("WGSize") << "Selecting workgroup size " << totalInvocations << " for " << totalInvocations
                          << " total invocations. Resulting in 1 group";
        return totalInvocations;
    }

    logWARNING("WGSize") << "Could not find a work group size dividable by " << THREADS_PER_CORE <<"!";

    GLint maxInt;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,0,&maxInt);
    uint32_t max = static_cast<uint32_t>(maxInt);

    // search for the divider of "totalInvocations" that is smaller than
    uint32_t result=1;
    for(uint32_t i=1; i < max; i++)
    {
        // check if it devides totalInvocations
        if(totalInvocations % i == 0 && (totalInvocations/i) >= COMPUTE_CORES)
        {
            result = i;
        }
    }


    logINFO("WGSize") << "Selecting workgroup size " << result << " for " << totalInvocations
                      << " total invocations. Resulting in " << totalInvocations / result << " groups";
    return result;
}

#endif //MPUTILS_DATATYPES_H_H
