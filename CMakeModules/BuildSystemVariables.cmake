# Define system specific parameters:
# BUILD_SYSTEM_OS {win|linux}
# BUILD_SYSTEM_ARCH {amd64}
# BUILD_COMPILER {vc-14.3|gcc-8.3|abi-cxx11}
# BUILD_COMPILER_NAME {vc|${CMAKE_C_COMPILER}}
# BUILD_COMPILER_VERSION (Compiler name with max and min version numbers)
# BUILD_COMPILER_MAJOR_VERSION
# BUILD_COMPILER_MINOR_VERSION
# BUILD_SYSTEM_CANONICAL_NAME {${BUILD_PLATFORM}_${BUILD_COMPILER}} (e.g. win64_vc-14.3,
#     linux64_gcc-8.3, linux64_abi-cxx11)
# BUILD_TYPE {64}
# BUILD_HWOS {amd64-nt|amd64-linux}
# BUILD_PLATFORM {win64|linux64}

# Must be 64 bit
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only 64 bit builds are supported by this build system.")
endif()

# Must be supported OS and compiler pairing
if(WIN32)
    # MSVC is a synonym for WIN32 in this build system
    if(NOT MSVC)
        message(FATAL_ERROR "Only MSVC supported on Windows.")
    endif()
    # MSVC must be 14.0 or newer
    if(MSVC_VERSION LESS 1900)
        message(FATAL_ERROR "Unsupported MSVC version ${MSVC_VERSION}")
    endif()
elseif(UNIX)
    # UNIX is a synonym for CMAKE_COMPILER_IS_GNUCXX in this build system
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
        message(FATAL_ERROR "Only g++ supported on Linux.")
    endif()
else()
    # UNIX is also a synonym for (NOT WIN32) in this build system
    message(FATAL_ERROR "Only Windows and Linux systems supported.")
endif()

if(WIN32)
    set(BUILD_SYSTEM_OS "win")
    set(OS_TYPE "nt")
    set(BUILD_COMPILER_NAME "vc")
else()
    set(BUILD_SYSTEM_OS "linux")
    set(OS_TYPE "linux")
    set(BUILD_COMPILER_NAME "gcc")
endif()

set(BUILD_SYSTEM_ARCH "amd64")
set(BUILD_TYPE "64")
set(BUILD_PLATFORM "${BUILD_SYSTEM_OS}${BUILD_TYPE}")
set(BUILD_HWOS "${BUILD_SYSTEM_ARCH}-${OS_TYPE}")

# Get compiler info
if(MSVC)
    # VS 2022 is 14.3
    set(BUILD_COMPILER_VERSION_MAJOR "14")
    set(BUILD_COMPILER_VERSION_MINOR "3")
    # Put them together to form something like 9.0, 10.0, 7.1, etc.
    set(BUILD_COMPILER_VERSION "${BUILD_COMPILER_VERSION_MAJOR}.${BUILD_COMPILER_VERSION_MINOR}")
else()
    # Get compiler name and version (gcc and gcc-compatible compilers)
    execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE BUILD_COMPILER_VERSION)
    string(REGEX REPLACE ".*([0-9]\\.[0-9])\\.[0-9].*" "\\1" BUILD_COMPILER_VERSION ${BUILD_COMPILER_VERSION})
    # Extract major and minor version numbers
    string(REGEX REPLACE "([0-9]+)\\.[0-9]+" "\\1" BUILD_COMPILER_MAJOR_VERSION ${BUILD_COMPILER_VERSION})
    string(REGEX REPLACE "[0-9]+\\.([0-9]+)" "\\1" BUILD_COMPILER_MINOR_VERSION ${BUILD_COMPILER_VERSION})
endif()

# Set the suffix for libraries
if(CMAKE_COMPILER_IS_GNUCXX)
    # These strings help identify the 3rd party libraries to link to
    set(BUILD_COMPILER_VERSION "8.3")

    # Detect if the CXX11 ABI is even supported with this compiler
    include(DetectGnuCxx11Abi)
    detect_gnucxx11_abi(SUPPORTS_CXX11_ABI COMPILER_DEFINITIONS "-D_GLIBCXX_USE_CXX11_ABI=1" MESSAGE "Detecting default CXX ABI")

    # Initialize the "FORCE" value for vsi_detect_cxx_abi below
    set(EFFECTIVE_ABI_FORCE)

    # Present option to build with CXX11 ABI or not, ONLY to users where it matters
    if(SUPPORTS_CXX11_ABI AND CMAKE_COMPILER_IS_GNUCXX)
        # Allow environment variable to override default behavior, which is ON, if supported
        set(_DEFAULT_CXX11_ABI ON)
        if(DEFINED ENV{ENABLE_CXX11_ABI})
            set(_DEFAULT_CXX11_ABI $ENV{ENABLE_CXX11_ABI})
        endif()

        # Specify an option for CXX ABI
        option(ENABLE_CXX11_ABI "Use the new C++-11 ABI, which is not backwards compatible." ${_DEFAULT_CXX11_ABI})
        if(NOT ENABLE_CXX11_ABI)
            add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
        endif()

        # Detect changes in ENABLE_CXX11_ABI, and set ABI_FORCE to "FORCE" when it does, so
        # that the effective ABI can be redetected as needed.
        if(DEFINED PREVIOUS_ENABLE_CXX11_ABI AND NOT ENABLE_CXX11_ABI STREQUAL PREVIOUS_ENABLE_CXX11_ABI)
            set(EFFECTIVE_ABI_FORCE "FORCE")
        endif()
        set(PREVIOUS_ENABLE_CXX11_ABI ${ENABLE_CXX11_ABI} CACHE INTERNAL "Previous value of ENABLE_CXX11_ABI")
    endif()

    # Re-detect the effective ABI. 
    detect_gnucxx11_abi(EFFECTIVE_CXX_ABI USE_CURRENT_DEFINES ${EFFECTIVE_ABI_FORCE} MESSAGE "Detecting effective CXX ABI")
    if(EFFECTIVE_CXX_ABI)
        set(BUILD_COMPILER_NAME "abi")
        set(BUILD_COMPILER_VERSION "cxx11")
    endif()
endif()

set(BUILD_COMPILER "${BUILD_COMPILER_NAME}-${BUILD_COMPILER_VERSION}")
set(BUILD_SYSTEM_CANONICAL_NAME "${BUILD_PLATFORM}_${BUILD_COMPILER}")
