cmake_minimum_required(VERSION 3.3)
project(mpUtils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -save-temps")

# librarys
find_package( Threads ) # the systems thread lib

set(SOURCE_FILES main.cpp mpUtils.cpp Log/Log.cpp Cfg/CfgFile.cpp Log/LogStream.cpp Log/ConsoleSink.cpp Log/FileSink.cpp Log/SyslogSink.cpp)
add_executable(mpUtils ${SOURCE_FILES})
target_link_libraries( mpUtils ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries( mpUtils stdc++fs )