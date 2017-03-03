/*
 * mpUtils
 * main.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail: hendrik.schwanekamp@gmx.net
 *
 * mpUtils = my personal Utillities
 * A utility library for my personal c++ projects
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

#include "Stopwatch.h"
#include <iostream>
#include "Log.h"
#include <chrono>
#include "CfgFile.h"
#include <typeinfo>
#include <ctime>
#include "DeltaTimer.h"
#include <thread>
#include "Timer.h"
#include "AsyncTimer.h"
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable


using namespace mpu;
using namespace std;
using namespace std::chrono;

constexpr int numRuns = 10;
double dTime =0;

int main()
{
    CpuStopwatch timer;

    // eliminate dedicated call with do while loop if possible
    // improve performance steal stuff from here: https://www.codeproject.com/Articles/288827/g-log-An-efficient-asynchronous-logger-using-Cplus
    //TODO: make the log more beautiful
    //TODO: cut file position path
    //TODO: add log block or something to log unformatted stuff
    //TODO: make it possible to log to console and file

    //TODO: remove the SAVE_DELEt macro
    //TODO: make the line comments doxygen ones

//        Log myLog(LogPolicy::CONSOLE, LogLvl::ALL);

    for(int j = 0; j < numRuns; ++j)
    {
        Log myLog( LogPolicy::FILE, "/home/hendrik/test.log", LogLvl::ALL);
        timer.reset();
        for(int i=0; i<1000000; i++)
        {
        myLog(LogLvl::INFO, MPU_FILEPOS , "TEST") << "Hi, a log";
//            myLog.logMessage( "[TEST] Info Hi, this is a log @" MPU_FILEPOS,LogLvl::INFO);
            //logWARNING("TEST") << "Some log warning";

//        logERROR("MODULE_TEST") << "some stuff has happend";
//
//        logDEBUG("some stuff") << "some stuff is debugging stuff";
//        logDEBUG2("some stuff") << "more debugging stuff";
        }

        dTime += timer.getSeconds() / numRuns;
        myLog.close();
    }



    //myLog.close();
    cout << "It took me " << dTime << " seconds on average" << endl;
    return 0;
}