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

// functions of the Log class
//-------------------------------------------------------------------

/*
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
*/

Log::~Log()
{
    close();
    if(globalLog == this)
        globalLog = nullptr;
}

/*

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
*/

void Log::removeSink(int index)
{
    std::lock_guard<std::mutex lck(loggerMtx);
    printFunctions.erase( printFunctions.begin() + index);
}

void Log::close()
{
    // accept no more messages
    LogLvl oldLvl = logLvl;
    logLvl = LogLvl::NOLOG;

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

    // remove all sinks
    printFunctions.clear();
    logLvl = oldLvl;
}

void Log::logMessage(LogMessage* lm)
{
    if(!printFunctions.empty() && lm->lvl <= logLvl)
    {
        std::lock_guard<std::mutex> lck(queueMtx);
        messageQueue.push(lm);
        loggerCv.notify_one();
    }
}

LogStream Log::operator()(const LogLvl lvl, std::string&& sFilepos, std::string&& sModule)
{
    LogMessage* lm = new LogMessage;
    lm->lvl = lvl;
    lm->sFilePosition = std::move(sFilepos);
    lm->sModue = std::move(sModule);
    lm->threadId = std::this_thread::get_id();
    lm->timepoint = time(nullptr);

    return LogStream( (*this), lm);
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

            // print to all sinks
            for(auto &&function : printFunctions)
            {
                function(*msg);
            }

            delete msg;

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

            // print to all sinks
            for(auto &&function : printFunctions)
            {
                function(*msg);
            }

            delete(msg);

            queueLck.lock();
        }
    }
}

// static variables
Log* Log::globalLog = nullptr;
}