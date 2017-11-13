cmake_minimum_required(VERSION 3.3)

# set src files
set(SOURCE_FILES
    main.cpp
        )

# create target
add_executable(devTest ${SOURCE_FILES})

# link libraries
target_link_libraries(devTest mpUtils)

