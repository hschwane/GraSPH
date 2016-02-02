/*
 * mpUtils
 * mpLog.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Log class, which provides simple text file logging
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
#ifdef __linux__
    #include <syslog.h>
#endif
#include "mpUtils.h"
//--------------------

// defines
//--------------------

// macro to print file position
#define _mpu_mystr(x) _mpu_mystr2(x) //convert to string
#define _mpu_mystr2(x) #x
#define MPU_FILEPOS  __FILE__ " in line: "  _mpu_mystr(__LINE__)  " in function " + std::string(__PRETTY_FUNCTION__) // ausgeben von datei und zeile und funktion

// macros for simplified global logging
#define logFATAL_ERROR if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::fatal_error) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::error)
#define logERROR if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::error) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::error)
#define logWARNING if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::warning) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::warning)
#define logINFO if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::info) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::info)
#define logDEBUG if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::debug) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::debug)
#define logDEBUG1 if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::debug1) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::debug1)
#define logDEBUG2 if(mpu::Log::noGlobal() || mpu::Log::getGlobal().getLogLevel() < mpu::LogLvl::debug2) ; \
                    else mpu::Log::getGlobal()(MPU_FILEPOS,mpu::LogLvl::debug2)

//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

//-------------------------------------------------------------------
/**
 * enum LogLvl
 * enum to specify the log level1
 */
enum LogLvl // enum to specify log level
{
    invalid = 9999, // invalid is very high, so invalid messages are never logged
    all = 8,
    debug2 = 7,
    debug1 = 6,
    debug = 5,
    info = 4,
    warning = 3,
    error = 2,
    fatal_error = 1,
    nolog = 0
};
extern const std::string LogLvlToString[]; // loockup to transform Loglvl to string
extern const std::string LogLvlStringInvalid; // lookup to transform Loglvl to string

//-------------------------------------------------------------------
/**
 * enum LogPolicy
 * enum to specify the log policy
 */
enum LogPolicy
{
    none = 0,
    console = 1, //log to std out and err
    file = 2, // log to a text file
    syslog = 3, // log to the linux syslog (only on linux)
    custom = 4 // specify your own stream to log to
};
extern const std::string LogPolicyToString[]; // loockup to transform LogPolicy to string

// TODO: a new syslog stream class is needed for syslog logging

//-------------------------------------------------------------------
/**
 * class Log
 *
 * usage:
 */
class Log : public std::ostringstream
{
public:

    // constructors
    Log(LogPolicy policy = LogPolicy::none, const std::string &sFile = "", const std::string &sErrorFile = "", LogLvl lvl = LogLvl::info);
    Log(LogPolicy policy, std::ostream *out, std::ostream *err, LogLvl lvl = LogLvl::info);
#ifdef __linux__
    Log(LogPolicy policy, const std::string &sIdent, int iOption, int iFacility, LogLvl lvl = LogLvl::info);
#endif
    ~Log(); // destructor

    // open function to open/reopen the log
    void open(LogPolicy policy, const std::string &sFile = "", const std::string &sErrorFile = "");
    void open(LogPolicy policy, std::ostream *out, std::ostream *err);
#ifdef __linux__
    void open(LogPolicy policy, const std::string &sIdent, int iOption, int iFacility);
#endif

    void close(); // close the internal streams, is called automaticly before open and on destruction

    inline void flush(); // a new flush function to format "log style"

    // getter and setter
    void setLvl(LogLvl lvl) {currentLvl = lvl;} // set the level of the current flush
    const LogLvl getLvl() const {return currentLvl;} // get the level of the current flush
    void setLogLevel(LogLvl lvl) {logLvl = lvl;} // set the current log level
    const LogLvl getLogLevel() const {return logLvl;} // get the current log level
    void setTimeFormat(const std::string &sFormat) {sTimeFormat = sFormat;} // set the timestamp format (like strftime)
    const std::string &getSTimeFormat() const {return sTimeFormat;} // get the timestamp format
    const LogPolicy &getCurrentPolicy() const {return logPolicy;} // get the log policy
    void makeGlobal() {globalLog = this;}   // makes the current log global
    static Log &getGlobal() {return *globalLog;} // gets the global log
    static bool noGlobal() {return (globalLog == nullptr);} // checks if there is no global log set

    // operators
    inline Log &operator()(std::string s); // use the () operator to specify the current line
    inline Log &operator()(std::string s, LogLvl lvl); // use the () operator to specify the current line and current lvl
    template<typename T>
    inline Log &operator<<(const T &t);  // inherit the << operator from ostringstream

    // manipulator support
    typedef Log &(*_log_manip)(Log &); // define manipulator type
    inline Log &operator<<(_log_manip manip); // enable manipulators
    inline Log &operator<<(const LogLvl &level); // enable log level values to be used as manipulators

    // manipulators to set the level and flush then
    inline static Log &FATAL_ERROR(Log &log);
    inline static Log &ERROR(Log &log);
    inline static Log &WARNING(Log &log);
    inline static Log &INFO(Log &log);
    inline static Log &DEBUG(Log &log);
    inline static Log &DEBUG1(Log &log);
    inline static Log &DEBUG2(Log &log);

private:

    LogLvl logLvl; // the log level
    LogLvl currentLvl; // level of the current message
    LogPolicy logPolicy; // the log policy

    std::ostream *outStream; // the stream we print our log on
    std::ostream *errorStream; // you can put errors in a different stream

    std::string sTimeFormat; // format of the timestamp
    std::string sCurrentLine; // this is appended to every log entry on debug level eg "@main.cpp in line 11 int main()"

    static Log *globalLog; // point this to the global log
};

// global functions
//--------------------

// my own endl and flush
inline Log & endl(mpu::Log & out)
{
    //out.put('\n'); \n is added automatically when flushing the stream
    out.flush();
    return out;
}

inline Log & flush(mpu::Log & out)
{
    out.flush();
    return out;
}

inline Log & ends(mpu::Log & out)
{
    out.put('\0');
    return out;
}

// toString overloads
inline const std::string &toString(LogLvl lvl)
{
    return (lvl < 0 || lvl > LogLvl::all)
           ? LogLvlStringInvalid : LogLvlToString[lvl];
}

inline const std::string &toString(LogPolicy policy)
{
    return LogPolicyToString[policy];
}
//--------------------

// define all the inline functions of the class
//--------------------

inline void Log::flush()
{
    if (currentLvl <= logLvl && logPolicy != LogPolicy::none )
    {
        if (currentLvl == error)
        {
            (*errorStream) << "[" << timestamp(sTimeFormat) << "] "
            << toString(currentLvl) << ": "
            << str()
            << sCurrentLine
            << "\n";
            errorStream->flush();
        }
        else
        {
            (*outStream) << "[" << timestamp(sTimeFormat) << "] "
            << toString(currentLvl) << ": "
            << str()
            << sCurrentLine
            << "\n";
            outStream->flush();
        }
    }
    str("");
    sCurrentLine = "";
    currentLvl = LogLvl::invalid;
}

inline Log &Log::operator()(std::string s)
{
    if (logLvl >= LogLvl::debug)
        sCurrentLine = "\t\t@:" + s;
    return *this;
}

inline Log &Log::operator()(std::string s, LogLvl lvl)
{
    if (logLvl >= LogLvl::debug)
        sCurrentLine = "\t\t@:" + s;
    currentLvl = lvl;
    return *this;
}

template<typename T>
inline Log &Log::operator<<(const T &t)
{
    (*(std::ostringstream *) this) << t;
    return *this;
}

inline Log &Log::operator<<(_log_manip manip)
{
    return manip(*this);
}

inline Log &Log::operator<<(const LogLvl &level)
{
    currentLvl = level; return *this;
}

inline Log &Log::FATAL_ERROR(Log &log)
{
    log.setLvl(LogLvl::fatal_error);
    log.flush();
    return log;
}

inline Log &Log::ERROR(Log &log)
{
    log.setLvl(LogLvl::error);
    log.flush();
    return log;
}

inline Log &Log::WARNING(Log &log)
{
    log.setLvl(LogLvl::warning);
    log.flush();
    return log;
}

inline Log &Log::INFO(Log &log)
{
    log.setLvl(LogLvl::info);
    log.flush();
    return log;
}

inline Log &Log::DEBUG(Log &log)
{
    log.setLvl(LogLvl::debug);
    log.flush();
    return log;
}

inline Log &Log::DEBUG1(Log &log)
{
    log.setLvl(LogLvl::debug1);
    log.flush();
    return log;
}

inline Log &Log::DEBUG2(Log &log)
{
    log.setLvl(LogLvl::debug2);
    log.flush();
    return log;
}

//--------------------

}
// overload std::endl and others for use in the log class
//--------------------
namespace std {

inline mpu::Log & endl(mpu::Log & out)
{
    //out.put('\n'); \n is added automatically when flushing the stream
    out.flush();
    return out;
}

inline mpu::Log & flush(mpu::Log & out)
{
    out.flush();
    return out;
}

inline mpu::Log & ends(mpu::Log & out)
{
    out.put('\0');
    return out;
}

}
//--------------------


#endif //MPUTILS_MPLOG_H
