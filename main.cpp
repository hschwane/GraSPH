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
#include "CfgFile.h"
using namespace std;
using namespace std::chrono;

int main()
{
    // do some config test
    mpu::CfgFile cfg("/home/hendrik/test.cfg");

    auto t1 = high_resolution_clock::now();

    for(auto e : cfg.getConfigList())
    {
        cout << "[" << e.first << "]\n";
        for (auto  a : e.second) {
            cout << a.first << " = " << a.second << "\n";
        }
    }

    cout << "\n\n";

    for( auto e : cfg.getBlockMap("blub") )
    {
        cout << e.first << " = " << e.second << "\n";
    }

    cout << "\n\n";

    for( auto e : cfg.getBlockList() )
    {
        cout << e << "\n";
    }

    cout << "\n\n";

    cout << cfg.getValue<int>("blub","val1") << "\n";
    cout << cfg.getValue<string>("blub","valb") << "\n";
    cout << cfg.getValue<bool>("otherBlock","valb") << "\n";
    cout << cfg.getValue<float>("otherBlock","valc") << "\n";

    auto t2 = high_resolution_clock::now();

    std::cout << "It took me " <<  duration_cast<milliseconds>(t2 - t1).count() << " ms." << endl;

    return 0;
}