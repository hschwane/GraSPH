/*
 * mpUtils
 * SyslogSink.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the SyslogSink class
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include <syslog.h>
#include <sstream>
#include "SyslogSink.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// function definitions of the SyslogSink class
//-------------------------------------------------------------------
SyslogSink::SyslogSink(const std::string &sIdent, int facility)
{
    this->sIdent = sIdent;
    openlog(this->sIdent.c_str(), LOG_PID | LOG_NDELAY, facility);
}

SyslogSink::~SyslogSink()
{
    closelog();
}

void SyslogSink::operator()(const LogMessage &msg)
{
    // translate the log lvl
    int iPriority;
    switch (msg.lvl)
    {
        case LogLvl::FATAL_ERROR:
            iPriority = LOG_CRIT;
            break;
        case LogLvl::ERROR:
            iPriority = LOG_ERR;
            break;
        case LogLvl::WARNING:
            iPriority = LOG_WARNING;
            break;
        case LogLvl::INFO :
            iPriority = LOG_INFO;
            break;
        case LogLvl::DEBUG :
        case LogLvl::DEBUG2:
            iPriority = LOG_DEBUG;
            break;
        default:
            // if there is no valid log level just delete the message
            return;
    }

    std::ostringstream ss;
    ss <<  "[" << toString(msg.lvl) << "]";

    if(!msg.sModue.empty())
        ss << " (" << msg.sModue << "):";

    ss << " " << msg.sMessage
         << "    Thread: " << std::setbase(16) << msg.threadId << std::setbase(10);


    ::syslog(iPriority, "%s", ss.str().c_str());

}

}
