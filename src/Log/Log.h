/*
 * mpUtils
 * mpLog.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Log class, which provides logging to text files, syslog, or custom streams
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_MPLOG_H
#define MPUTILS_MPLOG_H

// includes
//--------------------
#include <sstream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <iostream>
#include <functional>
#include "../mpUtils.h"

//--------------------

// defines
//--------------------

// macro to print file position
#define _folders 2
#define _mpu_mystr(x) _mpu_mystr2(x) //convert to string
#define _mpu_mystr2(x) #x
#define MPU_FILEPOS  std::string(mpu::shortenPath( __FILE__ , _folders)) + " Line: "  _mpu_mystr(__LINE__)  " Function " + std::string(__PRETTY_FUNCTION__) // output file, line and function

// macros for simplified global logging

#define logFATAL_ERROR(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::FATAL_ERROR) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::FATAL_ERROR, MPU_FILEPOS, MODULE)
#define logERROR(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::ERROR) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::ERROR, MPU_FILEPOS, MODULE)
#define logWARNING(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::WARNING) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::WARNING, MPU_FILEPOS, MODULE)
#define logINFO(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::INFO) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::INFO, MPU_FILEPOS, MODULE)
#define assert_critical(TEST,MODULE,MESSAGE) if(!( TEST )){ logFATAL_ERROR(MODULE) << "Assert failed: " << (MESSAGE) ; throw std::runtime_error(MESSAGE);}

// debug is disabled on release build
#ifdef NDEBUG
    #define logDEBUG(MODULE) if(false) mpu::Log::getGlobal()(mpu::LogLvl::DEBUG, MPU_FILEPOS, MODULE)
    #define logDEBUG2(MODULE) if(false) mpu::Log::getGlobal()(mpu::LogLvl::DEBUG2, MPU_FILEPOS, MODULE)
    #define assert_true(TEST,MODULE,MESSAGE)
#else

    #define logDEBUG(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::DEBUG) ; \
                        else mpu::Log::getGlobal()(mpu::LogLvl::DEBUG, MPU_FILEPOS, MODULE)
    #define logDEBUG2(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::DEBUG2) ; \
                        else mpu::Log::getGlobal()(mpu::LogLvl::DEBUG2, MPU_FILEPOS, MODULE)
    #define assert_true(TEST,MODULE,MESSAGE) if(!( TEST )){ logERROR(MODULE) << "Assert failed: " << (MESSAGE) ; throw std::runtime_error(MESSAGE);}
#endif
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// forward declarations
//--------------------
class LogStream;
//--------------------

//-------------------------------------------------------------------
/**
 * enum LogLvl
 * enum to specify the log level1
 */
enum LogLvl // enum to specify log level
{
    INVALID = 9999, // invalid is very high, so invalid messages are never logged
    ALL = 7,
    DEBUG2 = 6,
    DEBUG = 5,
    INFO = 4,
    WARNING = 3,
    ERROR = 2,
    FATAL_ERROR = 1,
    NOLOG = 0
};
extern const std::string LogLvlToString[]; // lookup to transform Loglvl to string
extern const std::string LogLvlStringInvalid; // lookup to transform Loglvl to string

//-------------------------------------------------------------------
/**
 * struct LogMessage
 * struct to specify all elements of a log message
 */
struct LogMessage
{
    std::string sMessage;
    std::string sFilePosition;
    std::string sModue;
    LogLvl lvl;
    time_t timepoint;
    std::thread::id threadId;
};

//-------------------------------------------------------------------
/**
 * @class Log
 * provides flexible formatted logging to multiple costomizable outputs
 *
 * @usage:
 * To use the log create an object using the constructor.
 * You can add as many sinks as you like. Sinks contole where messages are writen and how they are
 * formatted.
 * There is a console log sink and a file log sink with logrotation.
 * You can write your own custom log sink by creating a function object which
 * accepts a const reference to an object of LogMessage.
 * See the existing sinks for reference.
 *
 * You can set the Log level with setLogLevel(). Only messages wih equal or higher priority will
 * be logged. To log a message you can use the "( ... )" function call operator and provide additional
 * parameters like the LogLevel of the mesage and then input text to the message using the "<<" operator.
 * The message is formatted and written to the Log automatically in a different thread.
 *
 * the Global log:
 * There is one additional feature called the global log. you can make a log global using makeGlobal().
 * Note that there can only be one global log at any time. You can use the macro definitions above
 * to write message to the global log like logERROR << "An Error!" << endl
 * The first log created is always made global automatically.
 * All other classes of the mpu library will use the global log to output error messages.
 *
 * Thread safety:
 * The class was developed with the goal to allow logging from all threads at the same time, as a
 * result the class is totally thread save. Messages from different threads are printed line after line.
 * Also all parameters can safely be changed from different threads.
 *
 */
class Log
{
public:
    // constructors
    template <class... SINKS>
    Log(LogLvl lvl, SINKS&&... sinks);

    ~Log(); // destructor

    template <class FIRST_SINK, class... OTHER_SINKS>
    void addSinks(FIRST_SINK&& sink, OTHER_SINKS&&... tail); // add a number of sinks
    void addSinks(){}
    void removeSink(int index); // removes a given sink (be carefull)
    void close(); // removes all sinks and closes the logger thread (queue is flushed), is called automatically before open and on destruction

    void logMessage(LogMessage* lm); // logs a message to the log

    // getter and setter
    void setLogLevel(LogLvl lvl) {logLvl = lvl;} // set the current log level
    LogLvl getLogLevel() const {return logLvl;} // get the current log level
    void makeGlobal() {globalLog = this;}   // makes the current log global
    static Log &getGlobal() {return *globalLog;} // gets the global log
    static bool noGlobal() {return (globalLog == nullptr);} // checks if there is no global log set

    // operators
    LogStream operator()(LogLvl lvl, std::string&& sFilepos ="", std::string&& sModule="");

    // make noncopyable and nonmoveable
    Log(const Log& that) = delete;
    Log& operator=(const Log& that) = delete;
    Log(Log &&that) = delete;
    Log& operator=(const Log&& that) = delete;

private:
    std::atomic<LogLvl> logLvl; // the log level

    static Log* globalLog; // point this to the global log

    std::queue< LogMessage*> messageQueue; // queue to collect messages from all threads

    // thread management
    std::mutex queueMtx;  // mutex to protect the queue
    std::mutex loggerMtx; // protect the logging operation
    std::condition_variable loggerCv; // cv to notify the logger when new messages arrive
    bool bShouldLoggerRun; // controle if the logger thread is running

    std::thread loggerMainThread; // the logger main thread
    void loggerMainfunc(); // the mainfunc of the second thread

    std::vector<std::function<void(const LogMessage& msg)>> printFunctions; // the funtion used to print a message to the log
};

// global functions
//--------------------
// toString overloads
constexpr const std::string &toString(LogLvl lvl)
{
    return (lvl < 0 || lvl > LogLvl::ALL)
           ? LogLvlStringInvalid : LogLvlToString[lvl];
}

/*
 * find end of string at compile time
 */
constexpr const char * const strEnd(const char * const str)
{
    return *str ? strEnd(str + 1) : str;
}

/*
 * shortence a path to contain at most "folders" folder at compile time
 */
constexpr const char * const processPath(const char * const start, const char * const end, const size_t folders, const size_t path_level = 0)
{
    return (start < end && *end != '/' && *end != '\\') ?
           processPath(start, end - 1, folders, path_level) :
           ((start < end && path_level < folders) ?
            processPath(start, end - 1, folders, path_level + 1) :
            (end + 1)
           );
}

/*
 * shortence a path to contain at most "folders" folder at compile time
 */
constexpr const char * const shortenPath(const char * const path, const size_t folders)
{
    return processPath(path, strEnd(path), folders);
}

//--------------------


//-------------------------------------------------------------------
// definitions of template functions of the log classs

template <class... SINKS>
Log::Log(LogLvl lvl, SINKS&&... sinks)
{
    logLvl = lvl;
    bShouldLoggerRun = false;

    addSinks(std::forward<SINKS>(sinks)...);

    // first log created is going to be global
    if (noGlobal())
        makeGlobal();
}

template <class FIRST_SINK, class... OTHER_SINKS>
void Log::addSinks(FIRST_SINK&& sink, OTHER_SINKS&&... tail)
{

    {
        std::lock_guard<std::mutex> lck(loggerMtx);
        printFunctions.push_back(makeFuncCopyable(std::forward<FIRST_SINK>(sink)));

        if(!bShouldLoggerRun)
        {
            bShouldLoggerRun = true;
            loggerMainThread = std::thread(&Log::loggerMainfunc, this);
        }
    }
    addSinks(std::forward<OTHER_SINKS>(tail)...);
}

}

// include forward declared classes
//--------------------
#include "LogStream.h"
//--------------------

#endif //MPUTILS_MPLOG_H
