/*
 * mpUtils
 * ConsoleSink.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the ConsoleSink class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_CONSOLESINK_H
#define MPUTILS_CONSOLESINK_H

// includes
//--------------------
#include "Log.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

//-------------------------------------------------------------------
/**
 * class ConsoleSink
 *
 * usage:
 * Create an instance of Console Sink and pass it to the log class to print log
 * messages to the standard output.
 *
 */
class ConsoleSink
{
public:
    void operator()( const LogMessage &msg);
private:
    constexpr int levelToColor(const LogLvl lvl);
};

}
#endif //MPUTILS_CONSOLESINK_H
