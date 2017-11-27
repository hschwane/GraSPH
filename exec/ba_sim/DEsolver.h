/*
 * mpUtils
 * DEsolver.h
 *
 * Define the interface used by Differential Equation Solvers.
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the DEsolver class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_DESOLVER_H
#define MPUTILS_DESOLVER_H

// includes
//--------------------
#include "Graphics/Graphics.h"
//--------------------

//-------------------------------------------------------------------
/**
 * @brief Flags to describe features of different differential equation solvers
 */
enum class DEsolverFlag : char
{
    IS_SELF_STARTING = 2,
};

DEsolverFlag operator|(const DEsolverFlag lhs, const DEsolverFlag rhs)
{
    return static_cast<DEsolverFlag>(static_cast<char>(lhs) | static_cast<char>(rhs));
}

DEsolverFlag operator&(const DEsolverFlag lhs, const DEsolverFlag rhs)
{
    return static_cast<DEsolverFlag>(static_cast<char>(lhs) & static_cast<char>(rhs));
}

//-------------------------------------------------------------------
/**
 * class DEsolver
 *
 * usage:
 * Use this class as a base class when implementing a new differential equation solver to simulate the particles.
 * In the constructor make sure to set all flags that describe your solver.
 *
 * First use setAccelerator to set the function used for calculating the acceleration of the particles.
 * The function should operate on the buffer currently bound to the PARTICLE_BUFFER_BINDING.
 * Then set the disired delta time with setDT() be aware that some solvers might not like it if you change dt
 * between simulation steps.
 * Now set the buffer containing the particles using setParticles() with a buffer object and the total number of particles.
 * Finally use advanceTime to advance the simulation.
 * Some solvers require you to use the start() function before performing the first step. You can check by calling isSelfStarting.
 *
 */
class DEsolver
{
public:
    virtual ~DEsolver()=default;

    virtual void setAccelerator(std::function<void(void)> accelerator)=0; //!< set the function used to calculate the acceleration
    virtual void setDT(double dt)=0; //!< set the timestep
    virtual void setParticles(mpu::gph::Buffer particleBufferm, uint32_t number)=0; //!< set the particle buffer to be simulated

    virtual void start(){}  //!< start the solver if it is not self starting
    virtual void advanceTime()=0; //!< advance the time by dt

    bool isSelfStarting(){ return testFlag(DEsolverFlag::IS_SELF_STARTING);} //!< check if the solver is self starting

private:
    bool testFlag(DEsolverFlag flag) { return static_cast<bool>((m_flags & flag));}
    const DEsolverFlag m_flags; //!< flags describing solver properties

};


#endif //MPUTILS_DESOLVER_H
