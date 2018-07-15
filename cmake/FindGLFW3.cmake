#
# Try to find GLFW3 library and include path.
# Once done this will define
#
# GLFW3_FOUND
# GLFW3_INCLUDE_PATH
# GLFW3_LIBRARY
# 

set(GLFW3_ROOT_ENV $ENV{GLFW3_ROOT})

IF (MINGW)
    FIND_PATH( GLFW3_INCLUDE_PATH GLFW/glfw3.h
        ${GLFW3_ROOT_ENV}/include
    )

    FIND_LIBRARY( GLFW3_LIBRARY
        NAMES glfw3
        PATHS
        ${GLFW3_ROOT_ENV}/lib-mingw
        ${GLFW3_ROOT_ENV}/lib
    )

ELSEIF (MSVC)
    FIND_PATH( GLFW3_INCLUDE_PATH GLFW/glfw3.h
        ${GLFW3_ROOT_ENV}/include
    )

    IF (MSVC10)
        FIND_LIBRARY( GLFW3_LIBRARY
            NAMES glfw3
            PATHS
            ${GLFW3_ROOT_ENV}/lib-msvc100
            ${GLFW3_ROOT_ENV}/lib
        )	
	ELSEIF (MSVC14)
        FIND_LIBRARY( GLFW3_LIBRARY
            NAMES glfw3
            PATHS
            ${GLFW3_ROOT_ENV}/lib-vc2015
            ${GLFW3_ROOT_ENV}/lib
        )
    ELSE()
        FIND_LIBRARY( GLFW3_LIBRARY
            NAMES glfw3
            PATHS
            ${GLFW3_ROOT_ENV}/lib-msvc110
            ${GLFW3_ROOT_ENV}/lib
        )
    ENDIF ()
ELSEIF(APPLE)

    FIND_PATH(GLFW3_INCLUDE_PATH GLFW/glfw3.h DOC "Path to GLFW include directory."
	HINTS ${GLFW3_ROOT_ENV}/include
	PATHS /usr/include /usr/local/include /opt/local/include
    )
    
    FIND_LIBRARY( GLFW3_LIBRARY
        NAMES libglfw3.a glfw3 glfw
        PATHS $ENV{GLFW3_ROOT_ENV}/build/src /usr/lib /usr/local/lib /opt/local/lib
    )

ELSE()
	FIND_PATH(GLFW3_INCLUDE_PATH GLFW/glfw3.h
	PATHS ${GLFW3_ROOT_ENV}/include /usr/include /usr/local/include ~/local/include
	)
	FIND_LIBRARY(GLFW3_LIBRARY
        NAMES glfw3 glfw
        PATHS ${GLFW3_ROOT_ENV}/lib /usr/lib /usr/local/lib ~/local/lib
	PATH_SUFFIXES dynamic) 
ENDIF ()



SET(GLFW3_FOUND "NO")
IF (GLFW3_INCLUDE_PATH AND GLFW3_LIBRARY)
	SET(GLFW3_LIBRARIES ${GLFW3_LIBRARY})
	SET(GLFW3_FOUND "YES")
    message("EXTERNAL LIBRARY 'GLFW3' FOUND")
ELSE()
    message("ERROR: EXTERNAL LIBRARY 'GLFW3' NOT FOUND")
ENDIF (GLFW3_INCLUDE_PATH AND GLFW3_LIBRARY)
