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
#include "ConsoleSink.h"
#include "FileSink.h"
#include "SyslogSink.h"
#include <chrono>

#include <typeinfo>
#include <ctime>
#include "DeltaTimer.h"
#include <thread>
#include "Timer.h"
#include "AsyncTimer.h"
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <syslog.h>


using namespace mpu;
using namespace std;
using namespace std::chrono;

constexpr int numRuns = 10;
double dTime =0;

int main()
{
    CpuStopwatch timer;

    //TODO: add log block or something to log unformatted stuff

    // TODO: remove the catch all in async timer
    // TODO: remove the SAVE_DELEt macro
    // TODO: change makros in constexpr
    // TODO: make the line comments doxygen ones

    // TODO: add network
    // TODO: add serial
    // TODO: add arg parser
    // TODO: json / xml parser / class serialization
    // TODO: make stuff windows and mac compatible
    // TODO: actually compile as library and other cmake stuff
    // TODO: folder structure and convenient end user include all includes
    // TODO maybe use lock free queue for logger

//        Log myLog(LogPolicy::CONSOLE, LogLvl::ALL);

        Log myLog( LogLvl::ALL, FileSink("/home/hendrik/test.log"), ConsoleSink());

        myLog(LogLvl::INFO, MPU_FILEPOS , "TEST") << "Hi, a log";
//            myLog.logMessage( "[TEST] Info Hi, this is a log @" MPU_FILEPOS,LogLvl::INFO);
            //logWARNING("TEST") << "Some log warning";

//        logERROR("MODULE_TEST") << "some stuff has happend";
//
//        logDEBUG("some stuff") << "some stuff is debugging stuff";
//        logDEBUG2("some stuff") << "more debugging stuff";




    myLog.close();
    cout << "It took me " << dTime << " seconds on average" << endl;
    return 0;
}