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
const std::string LogLvlToString[] = {"NOLOG", "FATAL_ERROR", "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG2",
                                      "ALL"};
const std::string LogLvlStringInvalid = "INVALID";
const std::string LogPolicyToString[] = {"NONE", "CONSOLE", "FILE", "SYSLOG", "CUSTOM"};


// functions of the Log class
//-------------------------------------------------------------------
Log::Log(LogPolicy policy, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    logPolicy = LogPolicy::NONE;
    outStream = nullptr;
    bShouldLoggerRun = false;

    if (policy != LogPolicy::NONE)
        open(policy, "");

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

Log::Log(LogPolicy policy, const std::string &sFile, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    logPolicy = LogPolicy::NONE;
    outStream = nullptr;
    bShouldLoggerRun = false;

    if (policy != LogPolicy::NONE)
        open(policy, sFile);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

Log::Log(LogPolicy policy, std::ostream *out, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    logPolicy = LogPolicy::NONE;
    outStream = nullptr;
    bShouldLoggerRun = false;

    open(policy, out);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

#ifdef __linux__

Log::Log(LogPolicy policy, const std::string &sIdent, int iFacility, LogLvl lvl)
{
    sTimeFormat = "%c";
    logLvl = lvl;
    logPolicy = LogPolicy::NONE;
    outStream = nullptr;
    bShouldLoggerRun = false;

    open(policy, sIdent, iFacility);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

#endif

Log::~Log()
{
    close();
    if(globalLog == this)
        globalLog = nullptr;
}

void Log::open(LogPolicy policy, const std::string &sFile)
{
    // close in case it is already opened
    if (logPolicy != LogPolicy::NONE)
        close();

    switch (policy)
    {
    case CONSOLE:
        outStream = &std::cout;
        break;

    case FILE:
        ownedStream.reset( new std::ofstream(sFile, std::ofstream::out | std::ofstream::app));
        outStream = ownedStream.get();

        if (!outStream || !dynamic_cast<std::ofstream *>(outStream)->is_open())
            throw std::runtime_error("Log: Could not open output file stream!");
        break;

    default:
        throw std::invalid_argument("Log: You called the wrong open function/constructor for your policy!");
    }

    logPolicy = policy;

    bShouldLoggerRun = true;
    loggerMainThread = std::thread( &Log::loggerMainfunc, this);
}

void Log::open(LogPolicy policy, std::ostream *out)
{
    // close in case it is already opened
    if (logPolicy != LogPolicy::NONE)
        close();

    if (policy != LogPolicy::CUSTOM)
        throw std::invalid_argument("Log: You called the wrong open function/constructor for your policy!");

    outStream = out;
    logPolicy = policy;

    bShouldLoggerRun = true;
    loggerMainThread = std::thread( &Log::loggerMainfunc, this);
}

#ifdef __linux__

void Log::open(LogPolicy policy, const std::string &sIdent, int iFacility)
{
    // close in case it is already opened
    if (logPolicy != LogPolicy::NONE)
        close();

    if (policy != LogPolicy::SYSLOG)
        throw std::invalid_argument("Log: You called the wrong open function/constructor for your policy!");

    // use the an ostream with the syslog streambuffer
    ownedStreambuff.reset(new SyslogStreambuf(sIdent, iFacility, this));
    ownedStream.reset( new std::ostream( ownedStreambuff.get()));
    outStream = ownedStream.get();

    // turn of timestamp, since syslog already provides a timestamp
    setTimeFormat("");
    logPolicy = policy;

    bShouldLoggerRun = true;
    loggerMainThread = std::thread( &Log::loggerMainfunc, this);
}

#endif

void Log::close()
{
    logPolicy = LogPolicy::NONE; // accept no more messages

    // wait for the logger to print all queued messages and join the thread
    std::unique_lock<std::mutex> lck(loggerMtx);
    //std::cout << messageQueue.size();
    if(bShouldLoggerRun)
    {
        bShouldLoggerRun = false;
        loggerCv.notify_one();
    }
    lck.unlock();
    if(loggerMainThread.joinable())
        loggerMainThread.join();
    lck.lock();

    outStream = nullptr; // we probably don't own it so don't delete it
    ownedStream = nullptr; // deletes the owned stream
    ownedStreambuff = nullptr; // delte custom streambuffers

    setTimeFormat("%c");
}

void Log::logMessage(const std::string &sMessage, LogLvl lvl)
{
    if(logPolicy != LogPolicy::NONE && lvl <= logLvl)
    {
        std::lock_guard<std::mutex> lck(queueMtx);
        messageQueue.emplace( sMessage, lvl);
        loggerCv.notify_one();
    }
}

LogStream Log::operator()(LogLvl lvl, std::string sFilepos, std::string sModule)
{
    std::lock_guard<std::mutex> lck(timeFormatMtx);
    return LogStream( (*this), timestamp(sTimeFormat), sModule, lvl, sFilepos);
}

void Log::loggerMainfunc()
{
    std::unique_lock<std::mutex> lck(loggerMtx);

    // make sure all pending messages are printed if we stop the logger
    // before this thread actual gets cpu time for the first time
    if(!bShouldLoggerRun)
    {
        std::unique_lock<std::mutex> queuLck(queueMtx);
        while(!messageQueue.empty())
        {
            auto msg= messageQueue.front();
            messageQueue.pop();
            queuLck.unlock();

            lastLvl = msg.second;
            *outStream << msg.first << std::endl;

            queuLck.lock();
        }
    }

    while(bShouldLoggerRun)
    {
        loggerCv.wait(lck);

        std::unique_lock<std::mutex> queuLck(queueMtx);
        while(!messageQueue.empty())
        {
            auto msg= messageQueue.front();
            messageQueue.pop();
            queuLck.unlock();

            lastLvl = msg.second;
            *outStream << msg.first << std::endl;

            queuLck.lock();
        }
    }
}

// static variables
Log* Log::globalLog = nullptr;
}