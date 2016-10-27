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


int main()
{
    SimpleStopwatch timer;

    //Log myLog( LogPolicy::FILE, "/home/hendrik/test.log");
    Log myLog( LogPolicy::CONSOLE, LogLvl::ALL);

    myLog(LogLvl::INFO, MPU_FILEPOS, "TEST") << "Hi, " << "this is " << "a log";
    logWARNING("TEST") << "Some log warning";
    
    logERROR("MODULE_TEST") << "some stuff has happend";

    logDEBUG("some stuff") << "some stuff is debugging stuff";
    logDEBUG2("some stuff") << "more debugging stuff";


    myLog.close();

    timer.pause();
    cout << "It took me " << timer.getSeconds() << " seconds" << endl;
    return 0;
}