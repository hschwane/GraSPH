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
    struct tm *timeStruct = localtime(&timeRaw);

    char a_cResult[sFormat.length() + 100];
    strftime(a_cResult, sFormat.length() + 100, sFormat.c_str(), timeStruct);
    return std::string(a_cResult);
}


bool isDirectory(std::string sPath)
{
    struct stat sb;
    return (stat(sPath.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
}

}