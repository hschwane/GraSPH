/*
 * mpUtils
 * mpUtils.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Defines some basic mpUtility functions.
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_MPUTILS_H
#define MPUTILS_MPUTILS_H

// includes
//--------------------
#include <string>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// defines
//--------------------
//--------------------

// typedefs
//--------------------
// make using timer, usw easier
typedef std::chrono::duration<int,std::ratio<60*60*24*365>> years;
typedef std::chrono::duration<int,std::ratio<60*60*24*7>> weeks;
typedef std::chrono::duration<int,std::ratio<60*60*24>> days;
typedef std::chrono::hours hours;
typedef std::chrono::minutes minutes;
typedef std::chrono::seconds seconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::microseconds microseconds;
typedef std::chrono::nanoseconds nanoseconds;
//--------------------

// some global functions
//--------------------
// aliases for std::this_thread functions
inline void yield() {std::this_thread::yield();}
inline void sleep_d(auto duration) {std::this_thread::sleep_for(duration);}
inline void sleep(int sec) {sleep_d(seconds(sec));}
inline void sleep_ms(int ms) {sleep_d(milliseconds(ms));}
inline void sleep_us(int us) {sleep_d(microseconds(us));}
inline void sleep_until(auto tp) {std::this_thread::sleep_until(tp);}

//--------------------
// some string helper functions

std::string timestamp(std::string sFormat = "%c"); //  get current timestamp as string

std::string &removeWhite(std::string &s); // removes whitespace from string changing the string itself and returning it
std::string &cutAfterFirst(std::string &s, const std::string &c, const std::string &sEscape = "", size_t pos = 0); // cuts the first found char in c after pos and everything after that from s stuff can be escaped by any of the chars in sEscape
size_t findFirstNotEscapedOf(const std::string &s, const std::string &c, size_t pos = 0, const std::string &sEscape = "\\"); // returns the position of the first char from c in s after pos which is not escaped by a char from sEscape
std::string &escapeString(std::string &s, std::string sToEscape, const char cEscapeChar = '\\'); // escapes all chars from sToEscape in s using cEscapeChar
std::string &unescapeString(std::string &s, const char cEscapeChar = '\\'); // removes all cEscapeChars from the string but allow the escapeChar

template<typename T>
T fromString(const std::string &s); // extract a value from a string, bool is extracted with std::boolalpha on, used on string, the whole string is returned, usable on any class with << / >> overload

template<typename T>
std::string toString(const T &v); // converts value to string, bool is extracted with std::boolalpha on, usable on any class with << / >> overload

template<typename F >
auto makeFuncCopyable( F&& f ); // makes a moveable functor copyable using a shared pointer
//--------------------


// global template function definitions
//--------------------
template<typename T>
T fromString(const std::string &s)
{
    T value;
    std::istringstream ss(s);
    ss >> value;
    return value;
}

template<typename>
bool fromString(const std::string &s)
{
    bool value;
    std::istringstream ss(s);
    ss >> std::boolalpha >> value;
    return value;
}

template<typename>
tm fromString(const std::string &s, const std::string& format)
{
    tm value;
    std::istringstream ss(s);
    ss >> std::get_time(&value,format.c_str());
    return value;
}

template<typename>
const std::string& fromString(const std::string &s)
{
    return s;
}

template<typename T>
std::string toString(const T &v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

template<typename>
std::string toString(const bool &v)
{
    std::ostringstream ss;
    ss << std::boolalpha << v;
    return ss.str();
}

template<typename>
const std::string& toString(const std::string &v)
{
    return v;
}

template<typename>
const std::string toString(const tm &v, const std::string& format)
{
    std::ostringstream ss;
    ss << std::put_time( &v,format.c_str());
    return ss.str();
}

template<typename F >
auto makeFuncCopyable( F&& f )
{
    auto spf = std::make_shared<F>(std::forward<F>(f) );
    return [spf](auto&&... args)->decltype(auto)
    {
        return (*spf)( decltype(args)(args)... );
    };
}

}
#endif //MPUTILS_MPUTILS_H
