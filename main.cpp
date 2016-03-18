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

using namespace std;
using namespace std::chrono;

int main()
{
    mpu::DeltaTimer timer;

    // do stuff
    int a=0;
    for (int i = 0; i < 100000; ++i)
    {
        a+=i;
    }
    std::cout << "It took me " << timer.getDeltaTime() << " seconds" << endl;
    // do more stuff
    for (int i = 0; i < 100000; ++i)
    {
        a+=i;
    }
    std::cout << "It took me " << timer.getDeltaTime() << " seconds" << endl;
    // do more stuff
    for (int i = 0; i < 100000; ++i)
    {
        a+=i;
    }

    std::cout << "It took me " << timer.getDeltaTime() << " seconds" << endl;
    return 0;
}