/*
 * mpUtils
 * ConsoleSink.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ConsoleSink class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "ConsoleSink.h"
#include <iostream>
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// function definitions of the ConsoleSink class
//-------------------------------------------------------------------
void ConsoleSink::operator()(const LogMessage &msg)
{
    std::ostream* os;
    if(msg.lvl <= LogLvl::ERROR)
        os = &std::cerr;
    else
        os = &std::cout;

    struct tm timeStruct;
#ifdef __linux__
    localtime_r(&msg.timepoint, &timeStruct);
#else
#error please implement this for your operating system
#endif

    *os << "\033[1;" << levelToColor(msg.lvl) << "m"
        << "[" << toString(msg.lvl) << "]" << "\033[m "
        << " [" << std::put_time( &timeStruct, "%x %X") << "]";

    if(!msg.sModue.empty())
        *os << " (" << msg.sModue << "):";

    *os << "\t" << msg.sMessage
        << "\tThread: " << std::setbase(16) << msg.threadId << std::setbase(10);

    if(!msg.sFilePosition.empty())
        *os << "\t@File: " << msg.sFilePosition;

    *os << std::endl;
}

constexpr int ConsoleSink::levelToColor(LogLvl lvl)
{
    switch(lvl)
    {
        case LogLvl::FATAL_ERROR:
        case LogLvl::ERROR:
            return 31;
        case LogLvl::WARNING:
            return 33;
        case LogLvl::INFO:
            return 32;
        case LogLvl::DEBUG:
        case LogLvl::DEBUG2:
            return 35;
        default:
            return 38;
    }
}

}
