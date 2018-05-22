cmake_minimum_required(VERSION 3.3)

# create project
project(oglTest)

# set src files
set(SOURCE_FILES
        main.cpp
        )

# find directories
set(PROJECT_SHADER_PATH "${CMAKE_CURRENT_LIST_DIR}/shader" CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_RESOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/resources" CACHE PATH "Project specific path. Set manually if it was not found.")

# set defines
add_definitions(-DPROJECT_SHADER_PATH="${PROJECT_SHADER_PATH}/")
add_definitions(-DROJECT_RESOURCE_PATH="${ROJECT_RESOURCE_PATH}/")

# create target
add_executable(oglTest ${SOURCE_FILES})

# link libraries
target_link_libraries(oglTest mpUtils)

