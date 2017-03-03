/*
 * mpUtils
 * mpUtils.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements some basic mpUtility functions.
 *
 * Copyright 2016 Hendrik Schwanekamp
 */

// includes
//--------------------
#include "mpUtils.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// general help functions
//-------------------------------------------------------------------

std::string timestamp(std::string sFormat)
{
    time_t timeRaw = time(NULL);
    struct tm timeStruct;
#ifdef __linux__
    localtime_r(&timeRaw, &timeStruct);
#else
#error please implement timestamp for your operating system
#endif
    char a_cResult[sFormat.length() + 100];
    strftime(a_cResult, sFormat.length() + 100, sFormat.c_str(), &timeStruct);
    return std::string(a_cResult);
}

std::string &removeWhite(std::string &s)
{
    s.erase(std::remove_if( s.begin(), s.end(), [](char c){ return (c =='\r' || c =='\t' || c == ' ' || c == '\n');}), s.end() );
    return s;
}

std::string &cutAfterFirst(std::string &s, const std::string &c, const std::string &sEscape, std::size_t pos)
{
    std::string sSearch = c + sEscape;
    size_t cut = s.find_first_of(sSearch, pos);
    while (cut != std::string::npos)
    {
        if (c.find_first_of(s[cut]) != std::string::npos) // check if this is a cutting char
        {
            s.erase(cut);
            break;
        }
        // if not it is the escape char, so we ignore the next char
        cut = s.find_first_of(sSearch, cut + 2);
    }

    return s;
}

std::size_t findFirstNotEscapedOf(const std::string &s, const std::string &c, std::size_t pos, const std::string &sEscape)
{
    std::string sSearch = c + sEscape;
    size_t search = s.find_first_of(sSearch, pos);
    while (search != std::string::npos)
    {
        if (c.find_first_of(s[search]) != std::string::npos) // check if this is a cutting char
            return search;
        // if not it is the escape char, so we ignore the next char
        search = s.find_first_of(sSearch, search + 2);
    }

    return std::string::npos;
}

std::string &escapeString(std::string &s, std::string sToEscape, const char cEscapeChar)
{
    sToEscape.push_back(cEscapeChar);
    size_t pos = s.find_first_of(sToEscape);

    while (pos != std::string::npos)
    {
        s.insert(pos, 1, cEscapeChar);
        pos = s.find_first_of(sToEscape, pos + 2);
    }

    return s;
}

std::string &unescapeString(std::string &s, const char cEscapeChar)
{
    size_t pos = s.find(cEscapeChar);
    while (pos != std::string::npos)
    {
        s.erase(pos, 1);
        pos = s.find(cEscapeChar, pos + 1);
    }

    return s;
}

}