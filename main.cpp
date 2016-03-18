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

using namespace std;
using namespace std::chrono;

int main()
{

    mpu::CpuStopwatch w1;
    mpu::SimpleStopwatch w2;

    w1.start();
    w2.pause();

    // do stuff
    int a=0;
    for (int i = 0; i < 100000; ++i)
    {
        a+=i;
    }

//    w.pause();
//
    // do more stuff
    for (int i = 0; i < 100000; ++i)
    {
        a+=i;
    }

    //    w.resume();

    // do more stuff
    for (int i = 0; i < 100000; ++i)
    {
        a+=i;
    }

    w1.pause();
    w2.pause();
    std::cout << "It took me " << w1.getSeconds() << " seconds" << endl;
    std::cout << "It took me " << w2.getSeconds() << " seconds" << endl;
    return 0;
}