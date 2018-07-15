#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLM_FOUND
# GLM_INCLUDE_PATH
# 

set(GLM_ROOT_ENV $ENV{GLM_ROOT})

FIND_PATH(GLM_INCLUDE_PATH glm/glm.hpp
	PATHS ${GLM_ROOT_ENV} ${GLM_ROOT_ENV}/include /usr/include /usr/local/include ~/local/include
)

SET(GLM_FOUND "NO")
IF (GLM_INCLUDE_PATH)
	SET(GLM_FOUND "YES")
    message("EXTERNAL LIBRARY 'GLM' FOUND")
	add_definitions(-DGLM_FORCE_RADIANS)
ELSE()
    message("ERROR: EXTERNAL LIBRARY 'GLM' NOT FOUND")
ENDIF (GLM_INCLUDE_PATH)
