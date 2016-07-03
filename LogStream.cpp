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

LogStream::LogStream(const LogStream &ls) : logger(ls.logger), sFilepos(ls.sFilepos), lvl(ls.lvl)
{
    str(ls.str());
}

LogStream::LogStream(Log &logger, const std::string &sTimestamp, const std::string &sModule, LogLvl lvl,
                      std::string sFilepos) : logger(logger), sFilepos(sFilepos), lvl(lvl)
{
    if(!sTimestamp.empty())
        *this << "[" << sTimestamp << "]";
    if(!sModule.empty())
        *this << "[" << sModule << "]";

    *this  << " " << toString(lvl) << ": ";
}

LogStream::~LogStream()
{
    if (!sFilepos.empty())
        *this << "\t\t@" << sFilepos;

    logger.logMessage(str(), lvl);
}

}