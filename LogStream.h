/*
 * mpUtils
 * LogStream.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the LogStream class, which is used by the Log class to enable stream like logging
 *
 * Copyright 2016 Hendrik Schwanekamp
 * 
 */

#ifndef MPUTILS_LOGSTREAM_H
#define MPUTILS_LOGSTREAM_H

// includes
//--------------------
#include <sstream>
#include <stdexcept>
#include "mpUtils.h"
#include "Log.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------


/**
 * class LogStream
 * 
 * usage:
 * The constructor is usually called from the "Log" class. Then you can log using <<. After the ";" the Logstream is
 * destroyed. It writes its message to the log in its destructor.
 *
 */
class LogStream : public std::ostringstream
{
public:
    LogStream(const LogStream& ls);
    LogStream(Log &logger, const std::string &sTimestamp, const std::string &sModule, LogLvl lvl, std::string sFilepos);
    ~LogStream();

private:
    LogLvl lvl;
    Log &logger;
    std::string sFilepos;
};

}
#endif //MPUTILS_LOGSTREAM_H
