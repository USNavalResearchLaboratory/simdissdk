# Define system specific parameters:
# BUILD_SYSTEM_OS {win|linux}
# BUILD_SYSTEM_ARCH {x86|amd64}
# BUILD_COMPILER {vc-10.0|vc-12.0|vc-14.0|vc-14.1|vc-14.2|gcc-4.4}
# DEPRECATED_BUILD_COMPILER {vc-10.0|vc-12.0|vc-14.0|vc-14.1|vc-14.2|gcc-4.4}
# BUILD_COMPILER_NAME {vc|${CMAKE_C_COMPILER}}
# BUILD_COMPILER_VERSION (Compiler name with max and min version numbers)
# DEPRECATED_BUILD_COMPILER_VERSION (Compiler name with max, min, and patch version numbers)
# BUILD_COMPILER_MAJOR_VERSION
# BUILD_COMPILER_MINOR_VERSION
# BUILD_SYSTEM_CANONICAL_NAME {${BUILD_PLATFORM}_${BUILD_COMPILER}}
# BUILD_SYSTEM_LIB_SUFFIX     (Similar to BUILD_SYSTEM_CANONICAL_NAME, but uses DEPRECATED_BUILD_COMPILER for GCC to match old style lib names)
# BUILD_TYPE {32|64}
# BUILD_HWOS {x86-nt|amd64-nt|amd64-linux}
# BUILD_PLATFORM {win32|win64|linux64}

# CMAKE_SYSTEM_PROCESSOR is set to the PROCESSOR_ARCHITECTURE value (eg x86 or AMD64) for Windows and uname -p result or UNIX/Linux
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} BUILD_SYSTEM_ARCH)
if(BUILD_SYSTEM_ARCH STREQUAL "x86_64")
    set(BUILD_SYSTEM_ARCH "amd64")
elseif(BUILD_SYSTEM_ARCH MATCHES "i?86")
    set(BUILD_SYSTEM_ARCH "x86")
endif()

# Assign 32 or 64
if(WIN32)
    if(CMAKE_CL_64)
        set(BUILD_TYPE "64")
        set(BUILD_SYSTEM_ARCH "amd64")
    else()
        set(BUILD_TYPE "32")
        set(BUILD_SYSTEM_ARCH "x86")
    endif()
elseif(UNIX)
    if(BUILD_SYSTEM_ARCH STREQUAL "amd64")
        set(BUILD_TYPE "64")
    else()
        set(BUILD_TYPE "32")
    endif()
endif()

# Get system name
if(WIN32)
    set(BUILD_SYSTEM_OS win)
elseif(UNIX)
    set(BUILD_SYSTEM_OS linux)
endif()

set(BUILD_PLATFORM "${BUILD_SYSTEM_OS}${BUILD_TYPE}")

# Set HWOS value
if(WIN32)
    set(OS_TYPE "nt")
else()
    set(OS_TYPE ${BUILD_SYSTEM_OS})
endif()
if(BUILD_SYSTEM_ARCH STREQUAL "amd64" AND NOT BUILD_TYPE STREQUAL "64")
    set(BUILD_HWOS "x86-${OS_TYPE}")
else()
    set(BUILD_HWOS "${BUILD_SYSTEM_ARCH}-${OS_TYPE}")
endif()

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
    set(DEPRECATED_BUILD_COMPILER_VERSION ${BUILD_COMPILER_VERSION})
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel" OR "${CMAKE_CXX_COMPILER}" MATCHES "clang\\+\\+$")
    # Intel compiler will use latest gcc build version for third party libraries
    set(BUILD_COMPILER_NAME gcc)
    set(BUILD_COMPILER_VERSION "8.3")
    set(DEPRECATED_BUILD_COMPILER_VERSION "8.3")
else()
    # Get compiler name and version (gcc and gcc-compatible compilers)
    exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE BUILD_COMPILER_VERSION)
    set(BUILD_COMPILER_NAME gcc)
    string(REGEX REPLACE ".*([0-9]\\.[0-9]\\.[0-9]).*" "\\1" DEPRECATED_BUILD_COMPILER_VERSION ${BUILD_COMPILER_VERSION})
    string(REGEX REPLACE ".*([0-9]\\.[0-9])\\.[0-9].*" "\\1" BUILD_COMPILER_VERSION ${BUILD_COMPILER_VERSION})
endif()

# Extract major and minor version numbers
string(REGEX REPLACE "([0-9]+)\\.[0-9]+" "\\1" BUILD_COMPILER_MAJOR_VERSION ${BUILD_COMPILER_VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)" "\\1" BUILD_COMPILER_MINOR_VERSION ${BUILD_COMPILER_VERSION})

# Set the suffix for libraries
if(NOT WIN32)
    # These strings help identify the 3rd party libraries to link to
    set(BUILD_COMPILER_VERSION "8.3")
    # Determine whether we're on RHEL6. If so, fall back to GCC 4.4 tag, else use newer 8.3
    if(EXISTS "/etc/redhat-release")
        file(READ "/etc/redhat-release" _REDHAT_RELEASE)
        if(_REDHAT_RELEASE MATCHES "release 6\\.")
            set(BUILD_COMPILER_VERSION "4.4")
        endif()
        unset(_REDHAT_RELEASE)
    endif()
    set(DEPRECATED_BUILD_COMPILER_VERSION "${BUILD_COMPILER_VERSION}")
endif()

set(BUILD_COMPILER "${BUILD_COMPILER_NAME}-${BUILD_COMPILER_VERSION}")
set(DEPRECATED_BUILD_COMPILER "${BUILD_COMPILER}")
set(BUILD_SYSTEM_CANONICAL_NAME "${BUILD_PLATFORM}_${BUILD_COMPILER}")
set(BUILD_SYSTEM_LIB_SUFFIX "${BUILD_PLATFORM}_${BUILD_COMPILER}")
