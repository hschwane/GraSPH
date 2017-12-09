cmake_minimum_required(VERSION 3.3)

# create project
project(ba_sim)

# set src files
set(SOURCE_FILES
        main.cpp
        ParticleSpawner.cpp
        ParticleRenderer.cpp
        DEsolver/SimpleDEsolver.cpp
        DEsolver/Leapfrog.cpp
        DEsolver/Verlet.cpp
        DEsolver/VelocityVerlet.cpp
#        DEsolver/RungeKutta4.cpp
        )

# find directories
set(PROJECT_SHADER_PATH "${CMAKE_CURRENT_LIST_DIR}/shader" CACHE PATH "Project specific path. Set manually if it was not found.")
set(PROJECT_RESOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/resources" CACHE PATH "Project specific path. Set manually if it was not found.")

# set defines
add_definitions(-DPROJECT_SHADER_PATH="${PROJECT_SHADER_PATH}/")
add_definitions(-DROJECT_RESOURCE_PATH="${ROJECT_RESOURCE_PATH}/")

# create target
add_executable(ba_sim ${SOURCE_FILES})

# link libraries
target_link_libraries(ba_sim mpUtils)

