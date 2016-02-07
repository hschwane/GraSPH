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
#include <sys/stat.h>
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

bool isDirectory(std::string sPath);    // test, if the directory exists
//--------------------

}
#endif //MPUTILS_MPUTILS_H
