/*
 * mpUtils
 * mpLog.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Log class, which provides logging to text files, syslog, or custom streams
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "Log.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// Variables for logToString
const std::string LogLvlToString[] = {"NOLOG", "FATAL_ERROR", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2",
                                      "ALL"};
const std::string LogLvlStringInvalid = "INVALID";
const std::string LogPolicyToString[] = {"NONE", "CONSOLE", "FILE", "SYSLOG", "CUSTOM"};


// functions of the Log class
//-------------------------------------------------------------------

Log::Log(LogPolicy policy, const std::string &sFile, const std::string &sErrorFile, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    currentLvl = LogLvl::invalid;
    logPolicy = LogPolicy::none;
    outStream = nullptr;
    errorStream = nullptr;

    if (policy != LogPolicy::none)
        open(policy, sFile, sErrorFile);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

Log::Log(LogPolicy policy, std::ostream *out, std::ostream *err, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    currentLvl = LogLvl::invalid;
    logPolicy = LogPolicy::none;
    outStream = nullptr;
    errorStream = nullptr;

    open(policy, out, err);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

#ifdef __linux__

Log::Log(LogPolicy policy, const std::string &sIdent, int iFacility, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    currentLvl = LogLvl::invalid;
    logPolicy = LogPolicy::none;
    outStream = nullptr;
    errorStream = nullptr;

    open(policy, sIdent, iFacility);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

#endif

Log::~Log()
{
    close();
}

void Log::open(LogPolicy policy, const std::string &sFile, const std::string &sErrorFile)
{
    // close in case it is already opened
    if (logPolicy != LogPolicy::none)
        close();

    switch (policy)
    {
    case console:
        outStream = &std::cout;
        errorStream = &std::cerr;
        break;

    case file:
        outStream = new std::ofstream(sFile, std::ofstream::out | std::ofstream::app);
        if (sErrorFile.empty())
            errorStream = outStream;
        else
            errorStream = new std::ofstream(sErrorFile, std::ofstream::out | std::ofstream::app);

        if (!outStream || !dynamic_cast<std::ofstream *>(outStream)->is_open())
            throw std::runtime_error("Log: Could not open output file stream!");
        if (!errorStream || !dynamic_cast<std::ofstream *>(errorStream)->is_open())
            throw std::runtime_error("Log: Could not open output file stream!");
        break;

    default:
        throw std::invalid_argument("Log: You called the wrong open function/constructor for your policy!");
    }

    logPolicy = policy;
}

void Log::open(LogPolicy policy, std::ostream *out, std::ostream *err)
{
    // close in case it is already opened
    if (logPolicy != LogPolicy::none)
        close();

    if (policy != LogPolicy::custom)
        throw std::invalid_argument("Log: You called the wrong open function/constructor for your policy!");

    outStream = out;

    if (err)
        errorStream = err;
    else
        errorStream = out;

    logPolicy = policy;
}

#ifdef __linux__

void Log::open(LogPolicy policy, const std::string &sIdent, int iFacility)
{
    // close in case it is already opened
    if (logPolicy != LogPolicy::none)
        close();

    if (policy != LogPolicy::syslog)
        throw std::invalid_argument("Log: You called the wrong open function/constructor for your policy!");

    // use the an ostream with the syslog streambuffer
    outStream = new std::ostream(new SyslogStreambuf(sIdent, iFacility, this));
    errorStream = outStream;

    // turn of timestamp, since syslog already provides a timestamp
    setTimeFormat("");
    logPolicy = policy;
}

#endif

void Log::close()
{
    flush();
    switch (logPolicy)
    {
    case file:
        dynamic_cast<std::ofstream *>(outStream)->close();
        dynamic_cast<std::ofstream *>(errorStream)->close();
        if (outStream == errorStream)
        {
            MPU_SAVE_DELETE(outStream);
            errorStream = nullptr;
        }
        else
        {
            MPU_SAVE_DELETE(outStream);
            MPU_SAVE_DELETE(errorStream);
        }
        break;
    case syslog:
        // reset the timestamp string
        setTimeFormat("%c");
        if (outStream == errorStream)
        {
            MPU_SAVE_DELETE(outStream);
            errorStream = nullptr;
        }
        else
        {
            MPU_SAVE_DELETE(outStream);
            MPU_SAVE_DELETE(errorStream);
        }
        break;
    case console:
    case custom:
        outStream = nullptr;
        errorStream = nullptr;
        break;
    default:
        break;
    }
    logPolicy = LogPolicy::none;
}

// static variables
Log *Log::globalLog = nullptr;

}