/*
 * mpUtils
 * Cpu_Clock.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * This Class is header only!
 * Implements the cpu_clock class, which works like a std::chrono::steady_clock
 * but it uses cpu time instead of real time.
 *
 * Copyright 2016 Hendrik Schwanekamp
 * 
 */

#ifndef MPUTILS_CPU_CLOCK_H
#define MPUTILS_CPU_CLOCK_H

// includes
//--------------------
#include <chrono>
#include <ctime>
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// typedefs
//--------------------
typedef std::chrono::duration< clock_t, std::ratio<1, CLOCKS_PER_SEC>> cpu_ticks; // duration in cpu clock ticks
//--------------------

/**
 * class cpu_clock
 *
 * usage:
 * This class can be used like the std::chrono::steady_clock to calculate time intervals, bur it uses cpu time
 * consumed by the program instead of real time. Its duration is given in the type std::clock_t and represents the
 * number of clock ticks.
 *
 */
class cpu_clock
{
public:
    typedef cpu_ticks duration;
    typedef duration::rep rep;
    typedef duration::period period;
    typedef std::chrono::time_point<cpu_clock> time_point;
    static const bool is_steady = false;

    static time_point now() noexcept
    {
        return time_point( duration( clock()));
    }
};

}

#endif //MPUTILS_CPU_CLOCK_H
