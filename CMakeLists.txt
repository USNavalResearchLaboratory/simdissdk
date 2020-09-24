cmake_minimum_required(VERSION 3.0)

# Append our CMakeModules to the CMAKE_MODULE_PATH for better include() support
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeImport")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtilities")
project(SIMDIS_SDK)

# Configure valgrind settings
find_program(MEMORYCHECK_COMMAND NAMES valgrind)
set(MEMORYCHECK_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/CMakeUtilities/valgrind.supp" CACHE FILEPATH "File that contains suppressions for the memory checker")
# Pick up CDash/CTest
option(ENABLE_CDASH_PROJECTS "Generate the CDash test projects" OFF)
option(ENABLE_UNIT_TESTING "Enable unit testing" ON)
if(ENABLE_CDASH_PROJECTS)
    include(CTest)
elseif(ENABLE_UNIT_TESTING)
    enable_testing()
endif()

# Be rather quiet by default
option(VERBOSE "Toggles the displaying of details" OFF)
# Install headers by default
option(INSTALL_HEADERS "Install the SDK headers when performing a make install" ON)
# Install 3rd party libraries by default
option(INSTALL_THIRDPARTY_LIBRARIES "Install 3rd party libraries (OSG/osgEarth/Qt) on make install" ON)

# The ENABLE_DEPRECATED_API option is used to include in API which is being phased out.
# If you can compile your applications with this turned off you are
# well-placed for compatibility with future versions.
option(ENABLE_DEPRECATED_SIMDISSDK_API "Include deprecated SDK code" ON)
if(ENABLE_DEPRECATED_SIMDISSDK_API)
    add_definitions(-DUSE_DEPRECATED_SIMDISSDK_API)
else()
    remove_definitions(-DUSE_DEPRECATED_SIMDISSDK_API)
endif()

# Static build for libraries?
option(SIMNOTIFY_SHARED "If off, simNotify SDK libraries are built statically; if on, simNotify SDK libraries are built dynamically" ON)
option(SIMCORE_SHARED "If off, simCore SDK libraries are built statically; if on, simCore SDK libraries are built dynamically" ON)
option(SIMDATA_SHARED "If off, simData SDK libraries are built statically; if on, simData SDK libraries are built dynamically" ON)
option(SIMVIS_SHARED "If off, simVis SDK libraries are built statically; if on, simVis SDK libraries are built dynamically" ON)
option(SIMUTIL_SHARED "If off, simUtil SDK libraries are built statically; if on, simUtil SDK libraries are built dynamically" ON)
option(SIMQT_SHARED "If off, simQt SDK libraries are built statically; if on, simQt SDK libraries are built dynamically" ON)
# Install the SDK libraries, shared or static
set(INSTALL_SIMDIS_SDK_LIBRARIES ON)

# Default to a release mode build if not specified
if(NOT MSVC AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Allow CMake to use folders in MSVC; turn off when using Express versions
option(ENABLE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
if(ENABLE_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)
else()
    set_property(GLOBAL PROPERTY USE_FOLDERS FALSE)
endif()

# Include base macros required by other macros
include(BuildSystemVariables)
include(EnableNewCxxFeatures)
include(HelperFunctions)
include(CheckOSGEarthMethodExists)

# Determine endian-ness of current system (CMake provided test)
# SDK_BIG_ENDIAN is set to 0 or 1 (false or true) depending on result of test
include(TestBigEndian)
test_big_endian(SDK_BIG_ENDIAN)

# Defines an add_executable that respects CMAKE_<CONFIGURATION>_POSTFIX for non-imported executables
include(AddExecutablePostfix)

# Set architecture identifiers
string(TOUPPER ${BUILD_SYSTEM_ARCH} ARCH)
add_definitions(-D${ARCH})

# Set system identifiers
if(WIN32)
    if(MSVC)
        add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)

        option(ENABLE_STATIC_ANALYSIS "Enable Visual Studio's static analyzer" OFF)
        if(ENABLE_STATIC_ANALYSIS)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /analyze")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")
        endif()

        option(WIN32_USE_MP "Set to ON to build SIMDIS SDK with the /MP option" ON)
        mark_as_advanced(WIN32_USE_MP)
        if(WIN32_USE_MP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        endif()
    endif()
else()
    # Export global symbols to combat .so dynamic_cast<> issues
    unique_append_string(CMAKE_EXE_LINKER_FLAGS "-Wl,-E")
    unique_append_string(CMAKE_SHARED_LINKER_FLAGS "-Wl,-E")
    unique_append_string(CMAKE_MODULE_LINKER_FLAGS "-Wl,-E")
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} CACHE STRING "Flags used during linking executables")
    set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} CACHE STRING "Flags used during linking shared libraries and DLLs")
    set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS} CACHE STRING "Flags used during linking modules")

    # Enable -fPIC for compatibility
    unique_append_string(CMAKE_CXX_FLAGS "-fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} CACHE STRING "Flags used by the compiler during builds." FORCE)
endif()

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a debug postfix, usually d for Windows")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")


# Project Dependencies

# Configure 3rd party search path

# Include specific defaults for building with public release environment (third party packs)
include(PublicDefaults)
# Include specific defaults for building with VSI environment
include(VsiDefaults)
# Build Release mode by default on Linux, unless otherwise specified
include(DefaultToReleaseBuildType)

# Copy the helper script into the build directory
configure_file("${PROJECT_SOURCE_DIR}/CMakeUtilities/ExperimentalSubmit.cmake" "${PROJECT_BINARY_DIR}/ExperimentalSubmit.cmake")
configure_file("${PROJECT_SOURCE_DIR}/CMakeUtilities/NightlySubmit.cmake" "${PROJECT_BINARY_DIR}/NightlySubmit.cmake")
configure_file("${PROJECT_SOURCE_DIR}/CMakeUtilities/CTestCustom.cmake" "${PROJECT_BINARY_DIR}/CTestCustom.cmake")

# Configure coverage if it exists
if(${BUILD_COMPILER_NAME} STREQUAL "gcc" AND COVERAGE_COMMAND)
    option(ENABLE_COVERAGE "Enable coverage testing through gcov" OFF)
    if(ENABLE_COVERAGE)
        unique_append_string(CMAKE_C_FLAGS_DEBUG "-fprofile-arcs -ftest-coverage")
        unique_append_string(CMAKE_CXX_FLAGS_DEBUG "-fprofile-arcs -ftest-coverage")
        unique_append_string(CMAKE_EXE_LINKER_FLAGS_DEBUG "-lgcov")
        # Need to manually set() the variables after unique_append_string
        set(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "Flags used by the compiler during builds." FORCE)
        set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} CACHE STRING "Flags used by the compiler during builds." FORCE)
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE STRING "Flags used by the debug linker during builds" FORCE)
        # Coverage tests must be run in debug mode
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build in Debug or Release mode" FORCE)
    endif()
endif()

# Setup third party dependencies


# OSG: allow the user to set OSG_DIR in CMAKE GUI and auto-populate all the OSG libraries:
set(OSG_DIR "" CACHE PATH "OpenSceneGraph root directory")
if(NOT ENV{OSG_DIR} AND NOT ENV{OSGDIR} AND NOT ENV{OSG_ROOT})
    set(ENV{OSG_DIR} ${OSG_DIR})
endif()
set(OSG_INCLUDE_DIR ${OSG_DIR}/include)

# Prepopulate the installation runtime and library directory to override SIMDIS defaults
set(INSTALLSETTINGS_RUNTIME_DIR "bin" CACHE STRING "Directory containing exectuables and DLLs; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
set(INSTALLSETTINGS_LIBRARY_DIR "lib" CACHE STRING "Directory containing shared object files (UNIX only); non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
include(CreateInstallProperties)


# --- System Libraries --------------------
include(ImportSocket)
include(ImportOpenGL)

# ---Qt------------------------------------

# Attempt to find Qt so we can build the Qt example(s).
set(QT5_MODULES Designer OpenGL UiPlugin)
set(QT5_PLUGINS xcbglintegrations)
include(ImportQt5)


# --- OpenSceneGraph ----------------------

# OSG requires GDAL, and VSI-distributed GDAL requires MrSID
include(ImportMrSID)
include(ImportGDAL)

# import the package OSG and its companion OpenThreads
include(ImportOpenThreads)
include(ImportOSG)
include(ImportOSGQt)

## Find and include the OpenGL core header
# By default search for the GLCORE header in the OSG include path.  Non-default locations for the header
# can be set through the GLCORE_GLCOREARB_HEADER CMake cache variable
set(GLCORE_DIR ${OSG_LIBRARY_INCLUDE_PATH})
find_package(GLCORE)


# --- osgEarth -----------------------------

# VSI-distributed osgEarth requires GEOS
include(ImportGEOS)
# allow the user to set OSGEARTH_DIR in CMAKE GUI and auto-populate all OSGEARTH libraries:
set(OSGEARTH_DIR "" CACHE PATH "osgEarth root directory")
include(ImportOSGEarth)


# --- Protocol Buffers -----------------------------
set(PROTOBUF_DIR "" CACHE PATH "Protocol Buffers root directory")
include(ImportProtobuf)


# --- SQLITE3 -----------------------------
set(SQLITE3_DIR "" CACHE PATH "Sqlite3 root directory")
include(ImportSQLite)
if (SQLITE3_FOUND)
    set(SIM_HAVE_DB_SUPPORT 1)
endif()

# subprojects
add_subdirectory(SDK)
option(BUILD_SDK_EXAMPLES "Build SIMDIS SDK Example Applications" ON)
if(BUILD_SDK_EXAMPLES)
    add_subdirectory(Examples)
endif()
add_subdirectory(Plugins)
if(ENABLE_UNIT_TESTING)
    add_subdirectory(Testing)
endif()

# SWIG module requires Python3.  FindPython3.cmake is first distributed in CMake 3.12
if(NOT CMAKE_VERSION VERSION_LESS "3.12")
    option(BUILD_SDK_SWIG "Build SIMDIS SDK SWIG module" OFF)
    if(BUILD_SDK_SWIG)
        # Include SWIG for Python simCore module
        include(ImportSWIG)
        find_package(Python3 COMPONENTS Development Interpreter)
        add_subdirectory(swig)
    endif()
endif()

# Add documentation files
add_subdirectory(Doc)

# Configure the installs for CMake exports, e.g. for external projects
include(InstallCMakeExports)

# Install the documentation to the root of the install directory
install(FILES INSTALL.md LICENSE.txt README.md DESTINATION .)

# Mark several variables as advanced
include(SdkAdvancedVariables)
