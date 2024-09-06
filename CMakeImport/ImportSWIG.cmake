# Helper script for VSI to find SWIG executable.

initialize_ENV(SWIG_DIR)
set(SWIG_VERSION 4.2.1)
find_program(SWIG_EXECUTABLE
    NAMES swig
    HINTS
        $ENV{SWIG_DIR}/..
        $ENV{SWIG_DIR}/bin
        $ENV{SWIG_DIR}
        ${THIRD_DIR}/SWIG/${SWIG_VERSION}
        ${THIRD_DIR}/SWIG/${SWIG_VERSION}/bin
)
find_package(SWIG 4.0)

if(SWIG_FOUND)
    mark_as_advanced(FORCE SWIG_EXECUTABLE)
else()
    mark_as_advanced(CLEAR SWIG_EXECUTABLE)
endif()
