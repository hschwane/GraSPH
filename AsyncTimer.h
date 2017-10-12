/*
 * mpUtils
 * AsyncTimer.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the AsyncTimer class, which provides simple timer functionality similar to the Timer class, but running asynchronous.
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_ASYNCTIMER_H
#define MPUTILS_ASYNCTIMER_H

// includes
//--------------------
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Stopwatch.h"
#include "Cpu_Clock.h"
#include "Log.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

/**
 * class AsyncTimer
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
 * Start the timer with start(). The timer checks if it is finished in a new thread it manages itself and therefore works
 * asynchronously. You can also stop the timer manually which prevents the registered function from being called.
 * pause(), resume() and togglePause() can be used to pause the timer.
 *
 * Keep in mind that the registered function is also called in the other thread so make sure that all memory operations
 * this function does are thread safe.
 *
 * thread safety:
 * This class is totally thread safe and the same object can be modified from different threads
 * without causing data races.
 *
 * exceptions:
 * No Exceptions are thrown. If your registered function throws a exception it is caught and ignored.
 * Note that throwing exceptions from the registered function is not recommended.
 *
 */
template<typename clock>
class basic_AsyncTimer
{
public:

    // constructors
    basic_AsyncTimer();
    template<typename rep, typename periode>
    basic_AsyncTimer(std::chrono::duration<rep, periode> newDuration);
    template<typename rep, typename periode>
    basic_AsyncTimer(std::chrono::duration<rep, periode> newDuration, std::function<void()> func);
    template<typename rep, typename periode>
    basic_AsyncTimer(std::chrono::duration<rep, periode> newDuration, bool shouldLoop, std::function<void()> func);

    // destructor
    ~basic_AsyncTimer();

    template<typename rep, typename periode>
    void setDuration(std::chrono::duration<rep, periode> newDuration); // sets the time after which the timer finishes
    void setFunction(std::function<void()> func); // sets the function to be called when the timer finishes
    void setLooping(bool shouldLoop); // sets if the timer should be looped

    inline void start(); // start (restart if already running) the timer
    inline void stop(); // stop the timer

    inline void pause(); // pause the timer
    inline void resume(); // resume the timer
    inline void togglePause(); // toggle the pause state of the timer

    inline bool isRunning(); // check if the timer is running
private:
    typedef basic_Stopwatch<clock> stopwatch_type;
    typedef typename stopwatch_type::duration_type duration_type;
    stopwatch_type sw;

    bool bLooping;  // should the timer be looped
    bool bRunning;  // is the timer currently running?
    std::function<void()> finishFunction; // function to call when timer finishes
    duration_type timerDuration; // the duration the timer is going to run

    void update(); // the main function of the update thread
    std::thread timerThread;
    std::condition_variable cv;
    std::mutex mtx;
    std::mutex funcMtx;
};

// define all the of the basic_AsyncTimer class
//--------------------

template <typename clock>
basic_AsyncTimer<clock>::basic_AsyncTimer()
{
    bLooping = false;
    bRunning = false;
    timerDuration = duration_type(0);
}

template <typename clock>
template <typename rep, typename periode>
basic_AsyncTimer<clock>::basic_AsyncTimer(std::chrono::duration<rep, periode> newDuration)
{
    bLooping = false;
    bRunning = false;
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
}

template <typename clock>
template <typename rep, typename periode>
basic_AsyncTimer<clock>::basic_AsyncTimer(std::chrono::duration<rep, periode> newDuration, std::function<void()> func)
{
    bLooping = false;
    bRunning = false;
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
    finishFunction = func;
}

template <typename clock>
template <typename rep, typename periode>
basic_AsyncTimer<clock>::basic_AsyncTimer(std::chrono::duration<rep, periode> newDuration, bool shouldLoop,
                                std::function<void()> func)
{
    bLooping = shouldLoop;
    bRunning = false;
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
    finishFunction = func;
}

template <typename clock>
basic_AsyncTimer<clock>::~basic_AsyncTimer()
{
    stop();
}

template <typename clock>
void basic_AsyncTimer<clock>::setFunction(std::function<void()> func)
{
    std::lock_guard<std::mutex> funcLck(funcMtx);
    finishFunction = func;
}

template <typename clock>
template <typename rep, typename periode>
void basic_AsyncTimer<clock>::setDuration(std::chrono::duration<rep, periode> newDuration)
{
    std::unique_lock<std::mutex> lck(mtx);
    timerDuration = std::chrono::duration_cast<duration_type>(newDuration);
}

template <typename clock>
void basic_AsyncTimer<clock>::setLooping(bool shouldLoop)
{
    std::unique_lock<std::mutex> lck(mtx);
    bLooping = shouldLoop;
}

template <typename clock>
void basic_AsyncTimer<clock>::start()
{
    std::unique_lock<std::mutex> lck(mtx);
    if(bRunning)
    {
        bRunning = false;
        cv.notify_one();
    }
    lck.unlock();
    if(timerThread.joinable())
        timerThread.join();
    lck.lock();

    // now start the timer again
    bRunning = true;
    sw.reset();
    timerThread = std::thread( &basic_AsyncTimer<clock>::update, this);
}

template <typename clock>
void basic_AsyncTimer<clock>::stop()
{
    std::unique_lock<std::mutex> lck(mtx);
    if(bRunning)
    {
        bRunning = false;
        cv.notify_one();
    }
    lck.unlock(); // not sure if needed
    if(timerThread.joinable())
        timerThread.join();
}

template <typename clock>
void basic_AsyncTimer<clock>::pause()
{
    std::lock_guard<std::mutex> lck(mtx);
    sw.pause();
}

template <typename clock>
void basic_AsyncTimer<clock>::resume()
{
    std::lock_guard<std::mutex> lck(mtx);
    sw.resume();
}

template <typename clock>
void basic_AsyncTimer<clock>::togglePause()
{
    std::lock_guard<std::mutex> lck(mtx);
    sw.togglePause();
}

template <typename clock>
bool basic_AsyncTimer<clock>::isRunning()
{
    std::unique_lock<std::mutex> lck(mtx);
    return bRunning;
}

template <typename clock>
void basic_AsyncTimer<clock>::update()
{
    std::unique_lock<std::mutex> lck(mtx);

    while(bRunning) // if timer stops leave
    {
        cv.wait_for(lck, timerDuration-sw.getDuration()-microseconds(40)); // -40 us because timer seems more accurate this way

        if( !bRunning) // check if we were waken to stop the timer
            return;

        if(sw.getDuration() >= timerDuration) // check if the the wait is over
        {
            if(bLooping)
                sw.reset();
            else
                bRunning = false;

            lck.unlock(); // dont block while callback is running
            std::unique_lock<std::mutex> funcLck(funcMtx);
            try
            {
                if (finishFunction)
                    finishFunction();
            }
            catch (std::exception &e)
            {
                // log and rethrow
                logFATAL_ERROR("AsyncTimer") << "Exception in the timer function: "<<e.what();
                throw e;
            }
            funcLck.unlock();
            lck.lock();
        }
    }
    return;
}
//--------------------

// typedefs for the real async timer classes
//--------------------
typedef basic_AsyncTimer<std::chrono::steady_clock> SimpleAsyncTimer; // simple Stopwatch
typedef basic_AsyncTimer<std::chrono::high_resolution_clock> HRAsyncTimer; // high resolution Stopwatch
typedef basic_AsyncTimer<cpu_clock> CpuAsyncTimer; // uses cpu time

}
#endif //MPUTILS_ASYNCTIMER_H
