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
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include "mpUtils.h"

#ifdef __linux__
#include "SyslogStreambuf.h"
#endif
//--------------------

// defines
//--------------------

// macro to print file position
#define _mpu_mystr(x) _mpu_mystr2(x) //convert to string
#define _mpu_mystr2(x) #x
#define MPU_FILEPOS  __FILE__ " in line: "  _mpu_mystr(__LINE__)  " in function " + std::string(__PRETTY_FUNCTION__) // output file, line and function

// macros for simplified global logging

#define logFATAL_ERROR(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::FATAL_ERROR) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::FATAL_ERROR, MPU_FILEPOS, MODULE)
#define logERROR(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::ERROR) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::ERROR, MPU_FILEPOS, MODULE)
#define logWARNING(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::WARNING) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::WARNING, MPU_FILEPOS, MODULE)
#define logINFO(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::INFO) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::INFO, MPU_FILEPOS, MODULE)
#define logDEBUG(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::DEBUG) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::DEBUG, MPU_FILEPOS, MODULE)
#define logDEBUG2(MODULE) if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::DEBUG2) ; \
                    else mpu::Log::getGlobal()(mpu::LogLvl::DEBUG2, MPU_FILEPOS, MODULE)
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
 * enum LogPolicy
 * enum to specify the log policy
 */
enum LogPolicy
{
    NONE = 0,
    CONSOLE = 1, //log to std out and err
    FILE = 2, // log to a text file
    SYSLOG = 3, // log to the linux syslog (only on linux)
    CUSTOM = 4 // specify your own stream to log to
};
extern const std::string LogPolicyToString[]; // loockup to transform LogPolicy to string

//-------------------------------------------------------------------
/**
 * class Log
 * provides flexible formatted logging to stdout, files, the syslog, or custom streams.
 *
 * @usage:
 * To use the log create a object using one of the initialising constructors. you can also call the
 * empty constructor and initialise the log using ne of the open(...) functions.
 * Choose a log policy from the enum above and provide either file names, custom streams, or a
 * Identity string and a facility if you want to use syslog. If you provide only one stream or one
 * file, error and information will be send to the same output.
 *
 * You can set the global Log level with setLogLevel(). Only messages wih equal or higher priority will
 * be logged. To log a message you can use the "<<" operator. The message is formatted and
 * written to the Log when the stream is flushed. You also need to set the message lvl for each message
 * using setLvl() or streaming the lvl eg. "<< LogLvl::info" you could also use one of the
 * custom modifiers which add a level and then flush the stream.
 *
 * Note that redirecting anything to the Logs streambuffer will not work, since no Message lvl is
 * provided by other streams, nor will the buffer be flushed correctly.
 *
 * additional options:
 * You cn modify the timestamp format written to the file via setTimeFormat(string) where string is
 * the std library configuration string used for strftime.
 * If you want to output additional debug information you can add a string which is only outputted
 * if the log level is debug or higher. Use the function bracket operator like this:
 * myLog("@lineXY") << "hello" << DEBUG; to add a @lineXY. You can use the MPU_FILEPOS macro defined
 * above to get the current file, line and function name.
 *
 * the Global log:
 * there is one additional feature called the global log. you can make a log global using makeGlobal().
 * Note that there can only be one global log at any time. You can use the macro definitions above
 * to write message to the global log like logERROR << "An Error!" << endl
 * The first log created is always made global automatically.
 *
 * Thread safety:
 * Also streams are thread save on a per char basis, using the same Log from different files could result
 * in unreadable messages. You could however create multiple Log classes that write there output
 * to the same file.
 *
 * exceptions:
 * If initialisation fails, the constructor or the open(...) function will throw a exception.
 * No other exceptions are thrown.
 * 
 */
class Log
{
public:

    // constructors
    Log(LogPolicy policy = LogPolicy::NONE, const std::string &sFile = "", LogLvl lvl = LogLvl::INFO);
    Log(LogPolicy policy, std::ostream *out, LogLvl lvl = LogLvl::INFO);

#ifdef __linux__
    Log(LogPolicy policy, const std::string &sIdent, int iFacility, LogLvl lvl = LogLvl::INFO);
#endif

    ~Log(); // destructor

    // open function to open/reopen the log
    void open(LogPolicy policy, const std::string &sFile = "");
    void open(LogPolicy policy, std::ostream *out);

#ifdef __linux__
    void open(LogPolicy policy, const std::string &sIdent, int iFacility);
#endif

    void close(); // close the internal streams, is called automatically before open and on destruction

    void logMessage(const std::string &sMessage, LogLvl lvl); // logs a string as one message

    // getter and setter
    void setLogLevel(LogLvl lvl) {logLvl = lvl;} // set the current log level
    LogLvl getLogLevel() const {return logLvl;} // get the current log level
    void setTimeFormat(const std::string &sFormat) {std::lock_guard<std::mutex> lck(timeFormatMtx); sTimeFormat = sFormat;} // set the timestamp format (like strftime)
    std::string getTimeFormat() {std::lock_guard<std::mutex> lck(timeFormatMtx); return sTimeFormat;} // get the timestamp format
    LogPolicy getCurrentPolicy() const {return logPolicy;} // get the log policy
    void makeGlobal() {globalLog = this;}   // makes the current log global
    static Log &getGlobal() {return *globalLog;} // gets the global log
    static bool noGlobal() {return (globalLog == nullptr);} // checks if there is no global log set
    LogLvl getLastLvl() {return lastLvl;} // returns the log levelof the message that is currently written

    // operators
    LogStream operator()(LogLvl lvl, std::string sFilepos ="", std::string sModule="");

private:

    std::atomic<LogLvl> logLvl; // the log level
    std::atomic<LogLvl> lastLvl; // the log level of the message that is currently written
    std::string sTimeFormat; // format of the timestamp
    std::mutex timeFormatMtx;

    std::atomic<LogPolicy> logPolicy; // the log policy
    std::ostream *outStream; // the stream we print our log on

    static Log* globalLog; // point this to the global log

    std::queue< std::pair<std::string,LogLvl>> messageQueue; // queue to collect messages from all threads
    std::mutex queueMtx;  // mutex to protect the queue

    bool bShouldLoggerRun; // controle if the logger thread is running
    std::condition_variable loggerCv; // cv to notify the logger when new messages arrive
    std::mutex loggerMtx; // protect the logging operation

    std::thread loggerMainThread;

    void loggerMainfunc(); // the mainfunc of the second thread
    void startLogger(); // start the logger thread
};

// global functions
//--------------------
// toString overloads
inline const std::string &toString(LogLvl lvl)
{
    return (lvl < 0 || lvl > LogLvl::ALL)
           ? LogLvlStringInvalid : LogLvlToString[lvl];
}

inline const std::string &toString(LogPolicy policy)
{
    return LogPolicyToString[policy];
}
//--------------------

}

// include forward declared classes
//--------------------
#include "LogStream.h"
//--------------------

#endif //MPUTILS_MPLOG_H
