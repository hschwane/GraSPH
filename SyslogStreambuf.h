/*
 * mpUtils
 * SyslogStreambuf.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the SyslogStreambuf class, which provides a std::streambuffer that writes to
 * the Linux syslog. It is designed to be used by the Log class to Log to syslog.
 *
 * Copyright 2016 Hendrik Schwanekamp
 * 
 */

#ifdef __linux__ // only compile for linux

#ifndef MPUTILS_SYSLOGSTREAMBUF_H
#define MPUTILS_SYSLOGSTREAMBUF_H

// includes
//--------------------
#include <streambuf>
#include <string>
#include <syslog.h>
#include <stdexcept>
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// forward declarations
//--------------------
class Log;
//--------------------

// global function declarations
//--------------------
// (define this inline function outside os the namespace to prevent "syslog" from conflicting with mpu::LogLvl::syslog)
inline void __syslog(int iPriority, const std::string &sMessage);
//--------------------

/**
 * class SyslogStreambuf
 * a std::streambuffer which writes to syslog
 *
 * @usage:
 * This class is designed to provide syslog-write capability to a mpu::Log object.
 * Using it manually on an std::ostream is possible, but not recommended, since it
 * required a Log object to pull the current message level from.
 *
 */
class SyslogStreambuf : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
    SyslogStreambuf(const std::string &sIdent, int facility, Log *pLog); // constructor
    ~SyslogStreambuf(); //destructor

protected:
    int sync() override;   // here we write pending changes as a syslog message
    inline int overflow(int c) override; // put one char into the buffer
    inline std::streamsize xsputn(const char *s, std::streamsize n) override;

private:
    std::string sBuffer; // buffer the input, so we can write it to syslog on sync
    Log *pAssociatedLog; // pointer to the log we get our current priority / loglevel from

    std::string sIdent; // we need to save our name locally or some pointer reason
};

// define all the inline functions of the class
//--------------------
inline int SyslogStreambuf::overflow(int c)
{
    if (c != EOF)
    {
        sBuffer += static_cast<char>(c);
    }
    else
    {
        sync();
    }
    return c;
}

inline std::streamsize SyslogStreambuf::xsputn(const char *s, std::streamsize n)
{
    std::string st(s);
    if (st.find((char) EOF) == std::string::npos)
        sBuffer += st.substr(0, n);
    else
        for (int i = 0; i < st.length(); ++i)
            overflow(st[i]);
    return n;
}

}

// define this inline function outside os the namespace to prevent "syslog" from conflicting with mpu::LogLvl::syslog
inline void mpu::__syslog(int iPriority, const std::string &sMessage)
{
    syslog(iPriority, sMessage.c_str());
}

#endif //MPUTILS_SYSLOGSTREAMBUF_H
#endif // __ linux__