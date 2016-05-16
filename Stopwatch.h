/*
 * mpUtils
 * Stopwatch.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * This Class is header only!
 * Implements the Stopwatch class, which can measure Time like a stopwatch.
 *
 * Copyright 2016 Hendrik Schwanekamp
 * 
 */

#ifndef MPUTILS_STOPWATCH_H
#define MPUTILS_STOPWATCH_H

// includes
//--------------------
#include<chrono>
#include "Cpu_Clock.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------


/**
 * class basic_Stopwatch
 *
 * usage:
 *
 * Use this Class with a custom "clock" from std::chrono or use SimpleStopwatch, HRStopwatch or CpuStopwatch,
 * which use the steady_clock, high_resolution_clock and CpuStopwatch.
 *
 * The Stopwatch is started automatically on construction, you can manually restart it using start().
 * To read the time use getSeconds() or getDuration() (the second one returns a std::chrono::duration).
 * The timer is not stopped when you read the time (in fact it is never stopped, you caan only reset it using
 * start) so you can read the time multiple times from the same timer.
 * Using pause() and resume() you can pause the timer.
 *
 *
 */
template <typename clock>
class basic_Stopwatch
{
public:
    // some typedefs for clock type and time points and  durations
    typedef clock clock_type;
    typedef typename clock_type::time_point time_point_type;
    typedef typename clock_type::duration duration_type;

    basic_Stopwatch(); // constructor
    ~basic_Stopwatch(); // destructor

    inline void reset(); // resets the timer to zero
    inline void pause(); // pauses the timer
    inline void resume(); // resumes the timer
    inline void togglePause(); // toggles the pause state

    inline double getSeconds(); // returnes the time from start() to now as a double in seconds
    inline duration_type getDuration(); // returns the time start() to now as a std::chrono::duration

private:

    time_point_type startTime; // the time when start was called
    duration_type timeBeforePause; // time already mesired before pausing
    bool bPaused;
};

// define all the of the basic_Stopwatch class
//--------------------

template <typename clock>
basic_Stopwatch<clock>::basic_Stopwatch()
{
    reset();
}

template <typename clock>
basic_Stopwatch<clock>::~basic_Stopwatch()
{

}

template <typename clock>
void basic_Stopwatch<clock>::reset()
{
    timeBeforePause = duration_type(0);
    startTime = clock_type::now();
    bPaused = false;
}

template <typename clock>
double basic_Stopwatch<clock>::getSeconds()
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(getDuration()).count();
}

template <typename clock>
typename basic_Stopwatch<clock>::duration_type basic_Stopwatch<clock>::getDuration()
{
    if( bPaused)
        return timeBeforePause;

    return (clock_type::now() - startTime) + timeBeforePause;
}

template <typename clock>
void basic_Stopwatch<clock>::pause()
{
    timeBeforePause += clock_type::now() - startTime;
    bPaused = true;
}

template <typename clock>
void basic_Stopwatch<clock>::resume()
{
    startTime = clock_type::now();
    bPaused = false;
}

template <typename clock>
void basic_Stopwatch<clock>::togglePause()
{
    if(bPaused)
        resume();
    else
        pause();
}


//--------------------

// typedefs for the real stopwatch classes
//--------------------
typedef basic_Stopwatch<std::chrono::steady_clock> SimpleStopwatch; // simple Stopwatch
typedef basic_Stopwatch<std::chrono::high_resolution_clock> HRStopwatch; // high resolution Stopwatch
typedef basic_Stopwatch<cpu_clock> CpuStopwatch; // uses cpu time

}

#endif //MPUTILS_STOPWATCH_H
