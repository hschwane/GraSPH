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
#include <sys/stat.h>
#include <sstream>
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// defines
//--------------------
// macro to safely delete stuff on the heap
#define MPU_SAVE_DELETE(x) if( x != nullptr) {delete x; x = nullptr;}
//--------------------

// some global functions
//--------------------
std::string timestamp(std::string sFormat = "%c"); //  get current timestamp as string

inline std::string &removeWhite(std::string &s); // removes whitespace from string changing the string itself and returning it
std::string &cutAfterFirst(std::string &s, const std::string &c, const std::string &sEscape = "",
                           size_t pos = 0); // cuts the first found char in c after pos and everything after that from s stuff can be escaped by any of the chars in sEscape
size_t findFirstNotEscapedOf(const std::string &s, const std::string &c, size_t pos = 0,
                             const std::string &sEscape = "\\"); // returns the position of the first char from c in s after pos which is not escaped by a char from sEscape
std::string &escapeString(std::string &s, std::string sToEscape,
                          const char cEscapeChar = '\\'); // escapes all chars from sToEscape in s using cEscapeChar
std::string &unescapeString(std::string &s,
                            const char cEscapeChar = '\\'); // removes all cEscapeChars from the string but allow the escapeChar

template<typename T>
inline T fromString(
        const std::string &s); // extract a value from a string, bool is extracted with std::boolalpha on, used on string, the whole string is returned, usable on any class with << / >> overload

template<typename T>
inline std::string toString(
        const T &v); // converts value to string, bool is extracted with std::boolalpha on, usable on any class with << / >> overload

bool isDirectory(std::string sPath);    // test, if the directory exists
//--------------------

// global inline and template function definitions
//--------------------
inline std::string &removeWhite(std::string &s)
{
    s.erase(std::remove_if( s.begin(), s.end(), [](char c){ return (c =='\r' || c =='\t' || c == ' ' || c == '\n');}), s.end() );
    return s;
}

template<typename T>
inline T fromString(const std::string &s)
{
    T value;
    std::istringstream ss(s);
    ss >> std::boolalpha >> value;
    return value;
}

template<>
inline std::string fromString(const std::string &s)
{
    return s;
}

template<typename T>
inline std::string toString(const T &v)
{
    std::ostringstream ss;
    ss << std::boolalpha << v;
    return ss.str();
}

template<>
inline std::string toString(const std::string &v)
{
    return v;
}

}
#endif //MPUTILS_MPUTILS_H
