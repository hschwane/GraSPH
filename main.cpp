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

#include <iostream>
#include "Log.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

int main()
{
    // test the log
    mpu::Log myLog(mpu::LogPolicy::console);
    myLog.setLogLevel(mpu::debug);
    myLog.setTimeFormat("");

    myLog.open(mpu::syslog, "myTestApp", LOG_DAEMON);

    auto t1 = high_resolution_clock::now();

    logERROR << "TEST" << endl;
/*
    // lets send some messages
    for (int i = 0; i < 10; ++i)
    {
        logFATAL_ERROR << "This is a test debug Message. I is: " << i << " Lalalala" << endl;
        logERROR << "This is a test debug Message. I is: " << i << " Lalalala" << endl;
        logWARNING << "This is a test debug Message. I is: " << i << " Lalalala" << endl;
        logINFO << "This is a test debug Message. I is: " << i <<  " Lalalala" << endl;
        logDEBUG << "This is a test debug Message. I is: " << i <<  " Lalalala" << endl;
        logDEBUG1 << "This is a test debug Message. I is: " << i <<  " Lalalala" << endl;
        logDEBUG2 << "This is a test debug Message. I is: " << i <<  " Lalalala" << endl;
    }
*/

    auto t2 = high_resolution_clock::now();
    std::cout << "It took me " << duration_cast<milliseconds>(t2 - t1).count() << " ms." << endl;

    return 0;
}