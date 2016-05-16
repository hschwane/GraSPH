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

void func()
{
    cout << "thread"<<endl;
}

int main()
{
    SimpleStopwatch timer;
    SimpleAsyncTimer at(seconds(2));
    at.start();

    int i=0;
    while(at.isRunning())
    {
        i++;
    }

    timer.pause();
    cout << "counted to: "<<i<<endl;
    cout << "It took me " << timer.getSeconds() << " seconds" << endl;
    return 0;
}