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
#include "../Common.h"
//--------------------

//-------------------------------------------------------------------
/**
 * @brief Flags to describe features of different differential equation solvers
 */
enum class DEsolverFlag : char
{
    IS_SELF_STARTING = 2,
    REQUIRE_ONE_ACC = 4,
    REQUIRE_TWO_ACC = 8,
    REQUIRE_THREE_ACC = 16,
    REQUIRE_FOUR_ACC = 32,
};

inline DEsolverFlag operator|(const DEsolverFlag lhs, const DEsolverFlag rhs)
{
    return static_cast<DEsolverFlag>(static_cast<char>(lhs) | static_cast<char>(rhs));
}

inline DEsolverFlag operator&(const DEsolverFlag lhs, const DEsolverFlag rhs)
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
 * sync
 * The accelerator does not have to use any memory barriers after finishing the acceleration calculation.
 * Make sure to set the memory barrier yourself.
 *
 */
class DEsolver
{
public:
    DEsolver(DEsolverFlag flags) : m_flags(flags){}
    virtual ~DEsolver()=default;

    virtual void setAccelerator(std::function<void(void)> accelerator)=0; //!< set the function used to calculate the acceleration
    virtual void setDT(double dt)=0; //!< set the timestep
    virtual void setParticles(mpu::gph::Buffer particleBuffer, uint32_t number)=0; //!< set the particle buffer to be simulated

    virtual void start(){}  //!< start the solver if it is not self starting
    virtual void advanceTime()=0; //!< advance the time by dt

    bool isSelfStarting()const; //!< check if the solver is self starting
    int numberOfAccelerations()const; //!< check how many accelerations are computed per timestep

private:
    bool testFlag(DEsolverFlag flag) const;
    const DEsolverFlag m_flags; //!< flags describing solver properties
};

inline bool DEsolver::isSelfStarting() const
{
    return testFlag(DEsolverFlag::IS_SELF_STARTING);
}

inline int DEsolver::numberOfAccelerations() const
{
    if(testFlag(DEsolverFlag::REQUIRE_ONE_ACC))
        return 1;
    else if(testFlag(DEsolverFlag::REQUIRE_TWO_ACC))
        return 2;
    else if(testFlag(DEsolverFlag::REQUIRE_THREE_ACC))
        return 3;
    else if(testFlag(DEsolverFlag::REQUIRE_FOUR_ACC))
        return 4;
    else
        return 0;

}

inline bool DEsolver::testFlag(DEsolverFlag flag) const
{
    return static_cast<bool>((m_flags & flag));
}


#endif //MPUTILS_DESOLVER_H
