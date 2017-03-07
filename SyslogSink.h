/*
 * mpUtils
 * SyslogSink.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the SyslogSink class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_SYSLOGSINK_H
#define MPUTILS_SYSLOGSINK_H

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
 * class SyslogSink
 *
 * usage:
 *
 */
class SyslogSink
{
public:
    //constructor and destructor
    SyslogSink(const std::string &sIdent, int facility);
    ~SyslogSink();

    void operator()(const LogMessage &msg);
private:
    std::string sIdent; // we need to save the identity for some pointer reason

};

}

#endif //MPUTILS_SYSLOGSINK_H
