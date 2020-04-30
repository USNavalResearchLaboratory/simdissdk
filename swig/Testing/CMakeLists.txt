# Early return if no unit testing
if(NOT ENABLE_CDASH_PROJECTS AND NOT ENABLE_UNIT_TESTING)
    return()
endif()

# FindPython3.cmake was added in 3.12 and is better than PythonInterp
cmake_minimum_required(VERSION 3.12)

# Pull the version from CMakeImport/ImportPython3.cmake
set(DESIRED_PYTHON_VERSION ${Python3_VERSION})

if(WIN32)
    # Need to match the version exactly on Windows for some reason or crash
    find_package(Python3 ${DESIRED_PYTHON_VERSION} EXACT COMPONENTS Interpreter QUIET)
    # Python3's find_package() EXACT does not work as advertised in all cases.
    if(NOT DESIRED_PYTHON_VERSION VERSION_EQUAL Python3_VERSION)
        return()
    endif()
else()
    # Linux is more forgiving
    find_package(Python3 3.6 COMPONENTS Interpreter QUIET)
endif()

# Return if there's no interpreter
if(NOT Python3_Interpreter_FOUND)
    return()
endif()

# Add the tests
add_test(NAME simCore_SWIG_Test COMMAND "${Python3_EXECUTABLE}" "-B" "TestSimCore.py" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Windows insists on installing under a CONFIG directory under the runtime output directory
if(WIN32)
    set(PYTHON_PATH "${CMAKE_SWIG_OUTDIR}\;${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
else()
    set(PYTHON_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
endif()

set_tests_properties(
        simCore_SWIG_Test
    PROPERTIES
        ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
)
