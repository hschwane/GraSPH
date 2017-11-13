cmake_minimum_required(VERSION 3.3)

# set src files
set(SOURCE_FILES
    main.cpp
        ShaderTools.cpp
        )

# create target
add_executable(oglTest ${SOURCE_FILES})

# link libraries
target_link_libraries(oglTest mpUtils)

