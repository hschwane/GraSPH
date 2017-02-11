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
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// defines
//--------------------
// macro to safely delete stuff on the heap (dont use it, use smartpointers instead!!!)
#define MPU_SAVE_DELETE(x) if( x != nullptr) {delete x; x = nullptr;}

// macro to make the current thread sleep
#define MPU_YIELD() std::this_thread::yield();
#define MPU_SLEED_D(_DURATION_) std::this_thread::sleep_for(_DURATION_);
#define MPU_SLEEP(_SECONDS_) MPU_SLEED_D(mpu::seconds(_SECONDS_));
#define MPU_SLEEP_MS(_MILISECONDS_) MPU_SLEED_D(mpu::milliseconds(_MILISECONDS_));
#define MPU_SLEEP_US(_MICROSECONDS_) MPU_SLEED_D(mpu::microseconds(_MICROSECONDS_));
#define MPU_SLEEP_UNTIL(_TIMEPOINT_) MPU_SLEED_D(_TIMEPOINT_);
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

//--------------------

// global template function definitions
//--------------------
template<typename T>
T fromString(const std::string &s)
{
    T value;
    std::istringstream ss(s);
    ss >> std::boolalpha >> value;
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
    ss << std::boolalpha << v;
    return ss.str();
}

template<typename>
const std::string& toString(const std::string &v)
{
    return v;
}

}
#endif //MPUTILS_MPUTILS_H
