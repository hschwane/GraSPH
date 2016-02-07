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
// macro to savely delete stuff on the heap
#define MPU_SAVE_DELETE(x) if( x != nullptr) {delete x; x = nullptr;}
//--------------------

// some global functions
//--------------------
std::string timestamp(std::string sFormat = "%c"); //  get current timestamp as string

inline std::string &removeWhite(std::string &s); // removes whitespace from string changing the string itself and returning it
inline std::string &cutAfterFirst(std::string &s, const std::string &c, size_t pos = 0); // cuts the first found char in c after pos and everything after that from s
template<typename T>
inline T valueFromString(const std::string &s); // extract a value from a string, bool is extracted with std::boolalpha on, used on string, the whole string is returned
bool isDirectory(std::string sPath);    // test, if the directory exists
//--------------------

// global inline and template function definitions
//--------------------
inline std::string &removeWhite(std::string &s)
{
    s.erase(std::remove_if( s.begin(), s.end(), [](char c){ return (c =='\r' || c =='\t' || c == ' ' || c == '\n');}), s.end() );
    return s;
}

inline std::string &cutAfterFirst(std::string &s, const std::string &c, size_t pos)
{
    size_t cut = s.find_first_of(c,pos);
    if(cut != std::string::npos)
        s.erase(cut);
    return s;
}

template<typename T>
inline T valueFromString(const std::string &s)
{
    T value;
    std::istringstream ss(s);
    ss >> std::boolalpha >> value;
    return value;
}

template<>
inline std::string valueFromString(const std::string &s)
{
    return s;
}

}
#endif //MPUTILS_MPUTILS_H
