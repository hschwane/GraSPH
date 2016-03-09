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

    cfg.setValue("myBlock", "myKey1", 2.58610);
    cfg.setValue("myBlock2", "myKey2", 25);
    cfg.setValue("myBlock2", "myKey1", 80);
    cfg.setValue("myBlock", "myKey2", 2564);
    cfg.setValue("myBlock", "myKey3", "stuff");
    cfg.setValue("myBlock", "myKey4", "stuff with spaces");
    cfg.setValue("myBlock", "myKey5", "stuff with spaces and \\ and # and more");

    mpu::CfgFile::blockMap bl;
    bl["key"] = "value";
    bl["key2"] = "value2";
    bl["key3"] = "value3";

    cfg.addBlock("newBlock", bl);

    auto t2 = high_resolution_clock::now();

    std::cout << "It took me " << duration_cast<microseconds>(t2 - t1).count() << " us." << endl;

    return 0;
}