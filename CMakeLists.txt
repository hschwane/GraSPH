cmake_minimum_required(VERSION 3.3)

# Appends the cmake/ path to MAKE_MODULE_PATH variable.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# include some useful stuff
include(GNUInstallDirs)
include(GetVersionFromGit)

# create project
project(mpUtils VERSION "${VERSION_SHORT}")

# set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# find packages

# mutithreading
find_package( Threads ) # the systems thread lib

# openGL and window managing
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLM REQUIRED)

# search for src files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp) # yes i know, this is not recommended, but i still like it better and had no problems with it

message(${SOURCE_FILES})

# create target
add_library(mpUtils SHARED ${SOURCE_FILES})

# link libraries
target_link_libraries( mpUtils ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries( mpUtils stdc++fs )

# set properties
set_target_properties(mpUtils PROPERTIES VERSION ${VERSION_SHORT} SOVERSION ${VERSION_MAJOR})

# set include dir (in this case also the src)
target_include_directories(mpUtils PUBLIC src
        ${OPENGL_INCLUDE_DIRS}
        ${GLEW_INCLUDE_PATH}
        ${GLFW3_INCLUDE_PATH}
        ${GLM_INCLUDE_PATH}
        )
        #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
        #$<INSTALL_INTERFACE:src/>)

# link with external libs
target_link_libraries( mpUtils
        ${GLFW3_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${OPENGL_LIBRARY}
        )

# set install options
install(TARGETS mpUtils
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
        DIRECTORY ${CMAKE_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mpUtils
        FILES_MATCHING PATTERN "*.h*")

# see if there are executables and add the subdirectories
file(GLOB children RELATIVE ${CMAKE_SOURCE_DIR}/exec ${CMAKE_SOURCE_DIR}/exec/*)
set(subdirs "")
foreach(child ${children})
    if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/exec/${child})
        if (NOT ${child} MATCHES "\\..*")
            if(EXISTS ${CMAKE_SOURCE_DIR}/exec/${child}/CMakeLists.txt)
                string(REPLACE " " "_" child ${child})
                set(subdirs ${subdirs} ${child})
                message("Found Executable in folder '${child}'.")
            endif()
        endif()
    endif()
endforeach()
foreach(n ${subdirs})
    add_subdirectory(${CMAKE_SOURCE_DIR}/exec/${n})
endforeach()