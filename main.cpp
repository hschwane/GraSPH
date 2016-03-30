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


using namespace mpu;
using namespace std;
using namespace std::chrono;



int main()
{
    mpu::DeltaTimer timer;

    mpu::SimpleTimer t(seconds(5), [](){cout << "timer finished!!"<<endl;});
    t.start();

    while(t.update())
    {
;
    }

    std::cout << "It took me " << timer.getDeltaTime() << " seconds" << endl;
    return 0;
}