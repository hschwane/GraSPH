/*
 * mpUtils
 * LogStream.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the LogStream class, which is used by the Log class to enable stream like logging
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */


// includes
//--------------------
#include "LogStream.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// function definitions of the LogStream
//-------------------------------------------------------------------


LogStream::LogStream(LogStream&& other)  : logger(logger), lm(other.lm), std::ostringstream(std::move(other))
{

}

LogStream::LogStream(Log &logger, LogMessage* lm) : logger(logger), lm(lm)
{

}

LogStream::~LogStream()
{
    lm->sMessage = std::move(str());
    logger.logMessage(lm);
}

}