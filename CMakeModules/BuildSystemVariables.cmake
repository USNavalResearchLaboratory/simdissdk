# Define system specific parameters:
# BUILD_SYSTEM_NAME {win|linux}
# BUILD_SYSTEM_ARCH {x86|amd64}
# BUILD_TARGET_ARCH {x86|amd64}
# BUILD_SYSTEM_OS {nt|linux}
# BUILD_COMPILER {vc-9.0|vc-10.0|vc-11.0|vc-12.0|gcc-4.1|gcc-4.4}
# BUILD_COMPILER_NAME {vc|${CMAKE_C_COMPILER}}
# BUILD_COMPILER_VERSION 
# BUILD_COMPILER_MAJOR_VERSION
# BUILD_COMPILER_MINOR_VERSION
# BUILD_SYSTEM_LIB_SUFFIX
# BUILD_TYPE {32|64}
# BUILD_PLATFORM {win32|win64|linux32|linux64}

# CMAKE_SYSTEM_PROCESSOR is set to the PROCESSOR_ARCHITECTURE value (eg x86 or AMD64) for Windows and uname -p result or UNIX/Linux
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} BUILD_SYSTEM_ARCH)
if(BUILD_SYSTEM_ARCH STREQUAL "x86_64")
    set(BUILD_SYSTEM_ARCH "amd64")
elseif(BUILD_SYSTEM_ARCH MATCHES "i?86")
    set(BUILD_SYSTEM_ARCH "x86")
elseif(BUILD_SYSTEM_ARCH STREQUAL "sparc")
    set(BUILD_SYSTEM_ARCH "sparc")
endif(BUILD_SYSTEM_ARCH STREQUAL "x86_64")


# Assign 32 or 64
if(WIN32)
    if(CMAKE_CL_64)
        set(BUILD_TYPE "64")
        set(BUILD_SYSTEM_ARCH "amd64")
    else(CMAKE_CL_64)
        set(BUILD_TYPE "32")
        set(BUILD_SYSTEM_ARCH "x86")
    endif(CMAKE_CL_64)
elseif(UNIX)
    if(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND COMPILE_32_ON_64)
        set(BUILD_TYPE "32")
    else(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND COMPILE_32_ON_64)
        if(BUILD_SYSTEM_ARCH STREQUAL "x86")
            set(BUILD_TYPE "32")
        else(BUILD_SYSTEM_ARCH STREQUAL "x86")
            set(BUILD_TYPE "64")
        endif(BUILD_SYSTEM_ARCH STREQUAL "x86")
    endif(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND COMPILE_32_ON_64)
endif(WIN32)


# Get system name
if(WIN32)
    set(BUILD_SYSTEM_NAME win)
    set(BUILD_SYSTEM_OS nt)
elseif(UNIX)
    # CMAKE_SYSTEM_NAME contains value provided by 'uname -s'
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(BUILD_SYSTEM_NAME linux)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
        set(BUILD_SYSTEM_NAME solaris)
    endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BUILD_SYSTEM_OS ${BUILD_SYSTEM_NAME})
endif(WIN32)

set(BUILD_PLATFORM "${BUILD_SYSTEM_NAME}${BUILD_TYPE}")

# Get compiler info
if(MSVC)
    set(BUILD_COMPILER_NAME vc)
    # Calculate the build compiler version based on MSVC_VERSION:
    #   MSVC_VERSION : Compiler
    #           1600 : VC-10.0 (2010)
    #           1500 : VC-9.0 (2008)  ... etc
    # Major version, e.g. (1600 / 100) - 6 == (16) - 6 == 10
    # ... except this changes with VC 14 (MSVC 2015, 1900)
    if(MSVC_VERSION LESS 1900)
        math(EXPR BUILD_COMPILER_VERSION_MAJOR "( ${MSVC_VERSION} / 100 ) - 6")
    else()
        math(EXPR BUILD_COMPILER_VERSION_MAJOR "( ${MSVC_VERSION} / 100 ) - 5")
    endif()
    # Minor version, e.g. (1310 % 100) / 10 == (10) / 10 == 1
    math(EXPR BUILD_COMPILER_VERSION_MINOR "( ${MSVC_VERSION} % 100 ) / 10")
    # Put them together to form something like 9.0, 10.0, 7.1, etc.
    set(BUILD_COMPILER_VERSION "${BUILD_COMPILER_VERSION_MAJOR}.${BUILD_COMPILER_VERSION_MINOR}")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    # Intel compiler will use latest gcc build version for third party libraries
    set(BUILD_COMPILER_NAME gcc)
    set(BUILD_COMPILER_VERSION "4.1")
else(MSVC)
    # Get compiler name and version (gcc and gcc-compatible compilers)
    exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE BUILD_COMPILER_VERSION)
    string(REGEX REPLACE "([A-Za-z0-9])[ ].*" "\\1" BUILD_COMPILER_NAME ${BUILD_COMPILER_VERSION})
    string(REGEX REPLACE ".*([0-9]\\.[0-9]\\.[0-9]).*" "\\1" BUILD_COMPILER_VERSION ${BUILD_COMPILER_VERSION})
    set(BUILD_COMPILER_NAME gcc)
endif(MSVC)

# Extract major and minor version numbers
string(REGEX REPLACE "([0-9]+)\\.[0-9]+(\\.[0-9]+)?" "\\1" BUILD_COMPILER_MAJOR_VERSION ${BUILD_COMPILER_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)(\\.[0-9]+)?" "\\1" BUILD_COMPILER_MINOR_VERSION ${BUILD_COMPILER_VERSION})


# Set the suffix for libraries.  Currently 32 bit only expected.  
if(WIN32)
    set(BUILD_COMPILER "${BUILD_COMPILER_NAME}-${BUILD_COMPILER_VERSION}")
    set(BUILD_SYSTEM_LIB_SUFFIX "${BUILD_PLATFORM}_${BUILD_COMPILER}")
else(WIN32)
    # Solaris uses 3.4.6
    if(BUILD_COMPILER_MAJOR_VERSION EQUAL 3)
        set(BUILD_COMPILER_VERSION "3.4")
    elseif(BUILD_COMPILER_MAJOR_VERSION EQUAL 4)

        # Default to 4.1 unless we have good info otherwise
        set(BUILD_COMPILER_VERSION "4.1")

        # Detect g++ 4.4 preference over 4.1
        if(NOT BUILD_COMPILER_MINOR_VERSION EQUAL 1)
            set(DEFAULT_USE_GCC44 OFF)
            # Do a quick check on files to see if we should default on
            if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rd/linux64_gcc-4.4)
                if(VERBOSE)
                    message(STATUS "g++ 4.4 detected")
                endif(VERBOSE)
                set(DEFAULT_USE_GCC44 ON)
            endif()

            # Add an option to search for the 4.4 libraries
            option(COMPILE_GCC_4_4 "Enable g++ 4.4 3rd party libraries." ${DEFAULT_USE_GCC44})
            if(COMPILE_GCC_4_4)
                set(BUILD_COMPILER_VERSION "4.4")
            endif(COMPILE_GCC_4_4)
        endif(NOT BUILD_COMPILER_MINOR_VERSION EQUAL 1)

    else(BUILD_COMPILER_MAJOR_VERSION EQUAL 3)
        message(FATAL_ERROR "Unsupported compiler version ${BUILD_COMPILER_NAME} ${BUILD_COMPILER_VERSION}")
    endif(BUILD_COMPILER_MAJOR_VERSION EQUAL 3)
    set(BUILD_COMPILER "${BUILD_COMPILER_NAME}-${BUILD_COMPILER_VERSION}")
    set(BUILD_SYSTEM_LIB_SUFFIX "${BUILD_PLATFORM}_${BUILD_COMPILER}")
endif(WIN32)

# Trying library search based on compiler major and minor version...
set(BUILD_SYSTEM_LIB_SUFFIX_MAJOR_MINOR "${BUILD_PLATFORM}_${BUILD_COMPILER_NAME}-${BUILD_COMPILER_MAJOR_VERSION}\\.${BUILD_COMPILER_MINOR_VERSION}.*")

# Trying library search based on compiler major and version...
set(BUILD_SYSTEM_LIB_SUFFIX_MAJOR "${BUILD_PLATFORM}_${BUILD_COMPILER_NAME}-${BUILD_COMPILER_MAJOR_VERSION}.*")

# Enable GCC C++XX features
include(CMakeModules/EnableGccNewCxxFeatures.cmake)

if(VERBOSE)
    message(STATUS "Configuration Details:")
    message(STATUS "  BUILD_SYSTEM_NAME\t\t${BUILD_SYSTEM_NAME}")
    message(STATUS "  BUILD_SYSTEM_ARCH\t\t${BUILD_SYSTEM_ARCH}")
    message(STATUS "  BUILD_SYSTEM_OS\t\t${BUILD_SYSTEM_OS}")
    message(STATUS "  BUILD_COMPILER\t\t${BUILD_COMPILER}")
    message(STATUS "  BUILD_COMPILER_NAME\t\t${BUILD_COMPILER_NAME}")
    message(STATUS "  BUILD_COMPILER_VERSION\t${BUILD_COMPILER_VERSION}")
    message(STATUS "  BUILD_SYSTEM_LIB_SUFFIX\t${BUILD_SYSTEM_LIB_SUFFIX}")
    message(STATUS "  BUILD_TYPE\t\t${BUILD_TYPE}")
    message(STATUS "  BUILD_PLATFORM\t\t${BUILD_PLATFORM}")
    if(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND UNIX)
        if(COMPILE_32_ON_64)
            message(STATUS "  COMPILE_32_ON_64\t\tTrue")
        else(COMPILE_32_ON_64)
            message(STATUS "  COMPILE_32_ON_64\t\tFalse")
        endif(COMPILE_32_ON_64)
    endif(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND UNIX)
endif(VERBOSE)

include(CMakeModules/SdkAdvancedVariables.cmake)
