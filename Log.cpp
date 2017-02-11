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
#include <aliases.h>
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
    maxFileSize = 0;
    iNumLogsToKeep = 0;

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
    maxFileSize = 0;
    iNumLogsToKeep = 0;

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
    maxFileSize = 0;
    iNumLogsToKeep = 0;

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
    maxFileSize = 0;
    iNumLogsToKeep = 0;

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

    std::unique_lock<std::mutex> lck(loggerMtx);

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

        sLogfileName = sFile;
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

    std::unique_lock<std::mutex> lck(loggerMtx);

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

    std::unique_lock<std::mutex> lck(loggerMtx);

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

void Log::setupLogrotate(std::size_t maxFileSize, int numLogsToKeep)
{
    std::unique_lock<std::mutex> lck(loggerMtx);

    this->maxFileSize = maxFileSize;
    this->iNumLogsToKeep = numLogsToKeep;
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

            // check if we need to rotate the log
            if(logPolicy == LogPolicy::FILE && maxFileSize != 0 && ((std::size_t)(outStream->tellp()) + msg.first.size()) > maxFileSize)
            {
                namespace fs = std::experimental::filesystem;
                dynamic_cast<std::ofstream *>(outStream)->close();

                // rename all existing files deleting the oldest (if logs kept is zero or one this will not be executed at all)
                for(int i=iNumLogsToKeep-1; i >= 1; i--)
                {
                    if(fs::exists( sLogfileName + "." + toString(i)))
                        fs::rename( sLogfileName + "." + toString(i), sLogfileName + "." + toString(i+1));
                }

                // if we want to keep at least one, move the original
                if(iNumLogsToKeep > 0 && fs::exists( sLogfileName))
                    fs::rename( sLogfileName, sLogfileName + ".1");

                ownedStream.reset( new std::ofstream(sLogfileName, std::ofstream::out | std::ofstream::trunc));
                outStream = ownedStream.get();

                if (!outStream || !dynamic_cast<std::ofstream *>(outStream)->is_open())
                    throw std::runtime_error("Log: Could not open output file stream!");
            }

            lastLvl = msg.second;
            *outStream << msg.first << std::endl;

            queuLck.lock();
        }
    }

    while(bShouldLoggerRun)
    {
        loggerCv.wait(lck);

        std::unique_lock<std::mutex> queueLck(queueMtx);
        while(!messageQueue.empty())
        {
            auto msg= messageQueue.front();
            messageQueue.pop();
            queueLck.unlock();

            // check if we need to rotate the log
            if(logPolicy == LogPolicy::FILE && maxFileSize != 0 && ((std::size_t)(outStream->tellp()) + msg.first.size()) > maxFileSize)
            {
                namespace fs = std::experimental::filesystem;
                dynamic_cast<std::ofstream *>(outStream)->close();

                // rename all existing files deleting the oldest (if logs kept is zero or one this will not be executed at all)
                for(int i=iNumLogsToKeep-1; i >= 1; i--)
                {
                    if(fs::exists( sLogfileName + "." + toString(i)))
                        fs::rename( sLogfileName + "." + toString(i), sLogfileName + "." + toString(i+1));
                }

                // if we want to keep at least one, move the original
                if(iNumLogsToKeep > 0 && fs::exists( sLogfileName))
                    fs::rename( sLogfileName, sLogfileName + ".1");

                ownedStream.reset( new std::ofstream(sLogfileName, std::ofstream::out | std::ofstream::trunc));
                outStream = ownedStream.get();

                if (!outStream || !dynamic_cast<std::ofstream *>(outStream)->is_open())
                    throw std::runtime_error("Log: Could not open output file stream!");
            }

            lastLvl = msg.second;
            *outStream << msg.first << std::endl;

            queueLck.lock();
        }
    }
}

// static variables
Log* Log::globalLog = nullptr;
}