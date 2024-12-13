# Setup GLEW library
# Setting the GLEW_DIR environment variable will allow use of a custom built library

# Setup search paths
set(GLEW_VERSION 2.1.0)
initialize_ENV(GLEW_DIR)
set(INCLUDE_DIRS 
    $ENV{GLEW_DIR}/include
    ${THIRD_DIR}/GLEW/${GLEW_VERSION}/include
)

set(LIB_DIRS 
    $ENV{GLEW_DIR}/lib
    ${THIRD_DIR}/GLEW/${GLEW_VERSION}/lib
)


find_path(GLEW_LIBRARY_INCLUDE_PATH NAME GL/glew.h PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
find_library(GLEW_LIBRARY_DEBUG_NAME NAMES GLEW_d libGLEW_d PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
find_library(GLEW_LIBRARY_RELEASE_NAME NAMES GLEW libGLEW PATHS ${LIB_DIRS} NO_DEFAULT_PATH)

# Determine whether we found the library correctly
if(NOT GLEW_LIBRARY_RELEASE_NAME)
    set(GLEW_FOUND FALSE)
    mark_as_advanced(CLEAR GLEW_LIBRARY_INCLUDE_PATH GLEW_LIBRARY_DEBUG_NAME GLEW_LIBRARY_RELEASE_NAME)
    return()
endif()

mark_as_advanced(FORCE GLEW_LIBRARY_INCLUDE_PATH GLEW_LIBRARY_DEBUG_NAME GLEW_LIBRARY_RELEASE_NAME)
set(GLEW_FOUND TRUE)

# Set the release path, include path, and link libraries
add_library(GLEW STATIC IMPORTED)
set_target_properties(GLEW PROPERTIES
    IMPORTED_LOCATION "${GLEW_LIBRARY_RELEASE_NAME}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_LIBRARY_INCLUDE_PATH}"
    INTERFACE_COMPILE_DEFINITIONS "GLEW_STATIC"
    INTERFACE_LINK_LIBRARIES "VSI::GL"
)

# Set the debug path
if(GLEW_LIBRARY_DEBUG_NAME)
    set_target_properties(GLEW PROPERTIES
        IMPORTED_LOCATION_DEBUG "${GLEW_LIBRARY_DEBUG_NAME}"
    )
endif()

# Add library matching official name
add_library(GLEW::GLEW ALIAS GLEW)
