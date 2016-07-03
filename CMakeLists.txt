cmake_minimum_required(VERSION 3.3)
project(mpUtils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -save-temps")

# librarys
find_package( Threads ) # the systems thread lib

set(SOURCE_FILES main.cpp mpUtils.cpp Log.cpp SyslogStreambuf.cpp  CfgFile.cpp LogStream.cpp)
add_executable(mpUtils ${SOURCE_FILES})
target_link_libraries( mpUtils ${CMAKE_THREAD_LIBS_INIT} )