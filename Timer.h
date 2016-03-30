/*
 * mpUtils
 * Timer.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * This Class is header only!
 * Implements the Timer class, which provides simple timer functionality.
 *
 * Copyright 2016 Hendrik Schwanekamp
 * 
 */

#ifndef MPUTILS_TIMER_H
#define MPUTILS_TIMER_H

// includes
//--------------------
#include <chrono>
#include <functional>
#include "Stopwatch.h"
#include "Cpu_Clock.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

/**
 * class Timer
 * 
 * usage:
 *
 * Use this Class with a custom "clock" from std::chrono or use SimpleTimer, HRTimer or CpuTimer,
 * which use the steady_clock, high_resolution_clock and cpu_clock.
 *
 * You can set the time the timer should run with setDuration(), which you supply with any std::chrono::duration.
 * For easy use there are typedefs in mpUtils.h eg setDuration(seconds(1)).
 * If you want you can register a function to be called when the timer finishes or set the timer to looping which will make it
 * restart automatically.
 * Start the timer with start() and then call update repeatedly. In the update function the timer checks if it is finished
 * and change it's state. You can also stop the timer manually which prevents the registered function from being called.
 * pause(), resume() and togglePause() can be used to pause the timer.
 * However if you call update after the timer was paused it will stop the timer if the time limit was already reached before
 * the call to pause() happened.
 *
 * Keep in mind that the timers accuracy depends on how often you call update(). If you want the timer to update automatically
 * in the background use the AsyncTimer instead.
 *
 * thread safety:
 * Calling isReady from another thread should be fine, everything else is at own risk.
 * Especially calling update from another thread should be avoided, use AsyncTimer instead.
 *
 * exceptions:
 * Can throw std::bad_function_call if something is wrong with the function you provided to be called when the timer finishes.
 * (also if you throw something inside this function of cause)
 * Otherwise no exceptions are thrown.
 *
 */
template <typename clock>
class basic_Timer
{
public:

    // constructors
    basic_Timer();
    template <typename rep, typename periode>
    basic_Timer(std::chrono::duration<rep, periode> newDuration);
    template <typename rep, typename periode>
    basic_Timer(std::chrono::duration<rep, periode> newDuration, std::function<void()> func);
    template <typename rep, typename periode>
    basic_Timer(std::chrono::duration<rep, periode> newDuration, bool shouldLoop, std::function<void()> func);

    template <typename rep, typename periode>
    void setDuration(std::chrono::duration<rep, periode> newDuration); // sets the time after which the timer finishes
    void setFunction(std::function<void()> func); // sets the function to be called when the timer finishes
    void setLooping(bool shouldLoop); // sets if the timer should be looped

    inline void start(); // start (restart if already running) the timer
    inline void stop(); // stop the timer
    bool update(); // you need to call this repeatedly to check if the timer is finished returns false when the timer stopped running

    inline void pause(); // pause the timer
    inline void resume(); // resume the timer
    inline void togglePause(); // toggle the pause state of the timer

    inline bool isRunning(); // check if the timer is running
private:
    typedef basic_Stopwatch<clock> stopwatch_type;
    typedef typename stopwatch_type::duration_type duration_type;
    stopwatch_type sw;

    bool bLooping; // should the timer be looped
    bool bRunning;  // is the timer currently running?
    std::function<void()> finishFunction; // function to call when timer finishes
    duration_type timerDuration; // the duration the timer is going to run
};

// define all the of the basic_Stopwatch class
//--------------------

template <typename clock>
basic_Timer<clock>::basic_Timer()
{
    bLooping = false;
    bRunning = false;
    timerDuration = duration_type(0);
}

template <typename clock>
template <typename rep, typename periode>
basic_Timer<clock>::basic_Timer(std::chrono::duration<rep, periode> newDuration)
{
    bLooping = false;
    bRunning = false;
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
}

template <typename clock>
template <typename rep, typename periode>
basic_Timer<clock>::basic_Timer(std::chrono::duration<rep, periode> newDuration, std::function<void()> func)
{
    bLooping = false;
    bRunning = false;
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
    finishFunction = func;
}

template <typename clock>
template <typename rep, typename periode>
basic_Timer<clock>::basic_Timer(std::chrono::duration<rep, periode> newDuration, bool shouldLoop,
                                std::function<void()> func)
{
    bLooping = shouldLoop;
    bRunning = false;
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
    finishFunction = func;
}

template <typename clock>
void basic_Timer<clock>::setFunction(std::function<void()> func)
{
    finishFunction = func;
}

template <typename clock>
template <typename rep, typename periode>
void basic_Timer<clock>::setDuration(std::chrono::duration<rep, periode> newDuration)
{
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
}

template <typename clock>
void basic_Timer<clock>::setLooping(bool shouldLoop)
{
    bLooping = shouldLoop;
}

template <typename clock>
void basic_Timer<clock>::start()
{
    bRunning = true;
    sw.reset();
}

template <typename clock>
void basic_Timer<clock>::stop()
{
    bRunning = false;
}

template <typename clock>
bool basic_Timer<clock>::update()
{
    if(!bRunning)
        return false;

    if(sw.getDuration() >= timerDuration)
    {
        if(bLooping)
            sw.reset();
        else
            bRunning = false;

        if(finishFunction)
            finishFunction();
    }

    return isRunning();
}

template <typename clock>
void basic_Timer<clock>::pause()
{
    sw.pause();
}

template <typename clock>
void basic_Timer<clock>::resume()
{
    sw.resume();
}

template <typename clock>
void basic_Timer<clock>::togglePause()
{
    sw.togglePause();
}

template <typename clock>
bool basic_Timer<clock>::isRunning()
{
    return bRunning;
}
//--------------------

// typedefs for the real stopwatch classes
//--------------------
typedef basic_Timer<std::chrono::steady_clock> SimpleTimer; // simple Stopwatch
typedef basic_Timer<std::chrono::high_resolution_clock> HRTimer; // high resolution Stopwatch
typedef basic_Timer<cpu_clock> CpuTimer; // uses cpu time

}

#endif //MPUTILS_TIMER_H
