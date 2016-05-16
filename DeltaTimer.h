/*
 * mpUtils
 * DeltaTimer.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * This Class is header only!
 * Implements the DeltaTimer class, which provides a simple way to get the elapsed time since the last call
 * to a function.
 *
 * Copyright 2016 Hendrik Schwanekamp
 * 
 */

#ifndef MPUTILS_DELTATIMER_H
#define MPUTILS_DELTATIMER_H

// includes
//--------------------
#include<chrono>
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------


/**
 * class DeltaTimer
 * 
 * usage:
 * Calling getDeltaTime() returns the time elapsed since the previous call to delta time as a double in seconds.
 *
 */
class DeltaTimer
{
public:
    inline DeltaTimer();
    inline ~DeltaTimer();

    inline double getDeltaTime(); // returns the time since the last call to this function in seconds

private:
    std::chrono::steady_clock::time_point lastTime; // the time when getDeltaTime was called the first time
};

// define all the of the basic_Stopwatch class
//--------------------

DeltaTimer::DeltaTimer()
{
    lastTime = std::chrono::steady_clock::now();
}

DeltaTimer::~DeltaTimer()
{

}

double DeltaTimer::getDeltaTime()
{
    std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();
    double dt = std::chrono::duration_cast<std::chrono::duration<double>>( newTime-lastTime).count();
    lastTime = newTime;
    return dt;
}

//--------------------

}

#endif //MPUTILS_DELTATIMER_H
