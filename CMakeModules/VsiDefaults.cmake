if("$ENV{PEOPLE_DIR}" STREQUAL "")
    # Initialize empty values for the various _DIR parameters for end users without the 3rd party packs
    set(OSG_DIR "" CACHE PATH "OpenSceneGraph root directory")
    set(OSGEARTH_DIR "" CACHE PATH "osgEarth root directory")
    set(PROTOBUF_DIR "" CACHE PATH "protobuf root directory")
    set(SQLITE3_DIR "" CACHE PATH "SQLite root directory")
    set(GDAL_DIR "" CACHE PATH "3rd party library directory")
    return()
endif()

message(STATUS "VSI development environment detected; auto-configuring defaults.")

# Figure out the expected directory for a Qt package, set variables, and then find_package() it
macro(vsi_find_qt5_package PackageName FindPackage)
    set(EXPECTED_${PackageName}_SUBDIR "lib/cmake/${PackageName}")
    if(IS_DIRECTORY "${QTDIR}/${EXPECTED_${PackageName}_SUBDIR}")
        set(${PackageName}_DIR "${QTDIR}/${EXPECTED_${PackageName}_SUBDIR}" CACHE PATH "Qt5 ${PackageName} CMake path")
        mark_as_advanced(${PackageName}_DIR)
    elseif(IS_DIRECTORY "${DEFAULT_QT_LOCATION}/${EXPECTED_${PackageName}_SUBDIR}")
        set(${PackageName}_DIR "${DEFAULT_QT_LOCATION}/${EXPECTED_${PackageName}_SUBDIR}" CACHE PATH "Qt5 ${PackageName} CMake path")
        mark_as_advanced(${PackageName}_DIR)
    endif()
    if(${FindPackage} EQUAL "1")
        find_package(${PackageName})
    endif()
endmacro()

# Macro to configure Qt5 for VSI solutions; need to set Qt5Widgets_DIR and (for Windows) CMAKE_PREFIX_PATH
macro(vsi_configure_qt5)
    # Figure out the expected version and expected folder based on VSI layout
    set(EXPECTED_QT5_VERSION 5.5.1)
    set(DEFAULT_QT_LOCATION "c:/QtSDK/${BUILD_SYSTEM_LIB_SUFFIX}/${EXPECTED_QT5_VERSION}")
    if(NOT WIN32)
        set(DEFAULT_QT_LOCATION "/usr/local/Qt-${EXPECTED_QT5_VERSION}")
    endif()

    # Set up the variables for Qt5's Widgets package, but don't find_package it yet
    vsi_find_qt5_package(Qt5Widgets 0)

    # If all looks good here, set the CMAKE_PREFIX_PATH
    if(WIN32 AND IS_DIRECTORY "${Qt5Widgets_DIR}")
        # Need to be able to find glu32.lib -- different places for different builders
        if(CMAKE_CL_64)
            set(QT_GLU_PATHS
                "c:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64"
                "c:/Program Files/Microsoft SDKs/Windows/7.1A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/7.1A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/7.0A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/7.0A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/6.0A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/6.0A/Lib"
            )
        else()
            set(QT_GLU_PATHS
                "c:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x86"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/7.1A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/7.1A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/7.0A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/7.0A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/v7.0A/Lib"
                "c:/Program Files (x86)/Microsoft SDKs/Windows/6.0A/Lib"
                "c:/Program Files/Microsoft SDKs/Windows/6.0A/Lib"
            )
        endif()

        # Locate glu32 and set up the CMAKE_PREFIX_PATH
        find_path(QT_GLU_PATH glu32.lib PATHS ${QT_GLU_PATHS})
        if(NOT "${QT_GLU_PATH}" MATCHES "-NOTFOUND")
            set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${QT_GLU_PATH}" CACHE PATH "Prefix path, where to find glu32.lib")
            mark_as_advanced(QT_GLU_PATH CMAKE_PREFIX_PATH)
        endif()
    endif()
    
    # Set up the other search locations too
    if(IS_DIRECTORY "${Qt5Widgets_DIR}")
        vsi_find_qt5_package(Qt5Widgets 1)
        vsi_find_qt5_package(Qt5OpenGL 0)
        vsi_find_qt5_package(Qt5Designer 0)
        vsi_find_qt5_package(Qt5UiPlugin 0)

        # Set up the version numbers
        set(QT_VERSION ${Qt5Widgets_VERSION_STRING})
        string(REPLACE "." ";" QT_VERSION_LIST ${QT_VERSION})
        list(GET QT_VERSION_LIST 0 QT_VERSION_MAJOR)
        list(GET QT_VERSION_LIST 1 QT_VERSION_MINOR)
        list(GET QT_VERSION_LIST 2 QT_VERSION_PATCH)
        if(VERBOSE)
            message(STATUS "Found Qt5 version: ${QT_VERSION}")
        endif()

    endif()
endmacro()

macro(vsi_configure_qt4)
    # Point to the defined qmake executable
    if(NOT EXISTS "${QT_QMAKE_EXECUTABLE}")
        set(EXPECTED_VERSION 4.8.6)
        set(QMAKE_EXE qmake.exe)
        set(DEFAULT_QMAKE_LOCATION "c:/QtSDK/${BUILD_SYSTEM_LIB_SUFFIX}/${EXPECTED_VERSION}/bin/${QMAKE_EXE}")
        if(NOT WIN32)
            set(QMAKE_EXE "qmake")
            set(DEFAULT_QMAKE_LOCATION "/usr/local/Trolltech/Qt-${EXPECTED_VERSION}/bin/${QMAKE_EXE}")
        endif()
        # If QMake is where we think it is, use it
        if(EXISTS "${DEFAULT_QMAKE_LOCATION}")
            set(QT_QMAKE_EXECUTABLE "${DEFAULT_QMAKE_LOCATION}" CACHE FILE "The qmake executable for the Qt installation to use." FORCE)
        endif()
    endif()
endmacro()


# Update the THIRDPARTY_LIBRARY_DIR settings
if(IS_DIRECTORY "$ENV{PEOPLE_DIR}/../3rd/${BUILD_SYSTEM_LIB_SUFFIX}")
    set(THIRDPARTY_LIBRARY_DIR "$ENV{PEOPLE_DIR}/../3rd/${BUILD_SYSTEM_LIB_SUFFIX}" CACHE PATH "3rd party library directory")
else()
    set(THIRDPARTY_LIBRARY_DIR "$ENV{PEOPLE_DIR}/../3rd" CACHE PATH "3rd party library directory")
endif()

set(PROTOBUF_VERSION "2.6.0")
# Note that OSG 3.4.1 is used against Qt 5.5.1 builds internally
set(OSG_VERSION "3.4.1")
set(SQLITE_VERSION "3.8.11.1")
set(GDAL_VERSION "2.1.1")

# Set up Qt
option(ENABLE_QT5 "Attempt to build against Qt5 instead of Qt4" ON)
if(ENABLE_QT5)
    vsi_configure_qt5()
endif()

# Fall back to Qt4?
set(OSGEARTH_VERSION "OSG-${OSG_VERSION}")
if(IS_DIRECTORY "${Qt5Widgets_DIR}")
    # Our osgEarth directory is versioned with Qt- values for 5.x
    set(OSGEARTH_VERSION "OSG-${OSG_VERSION}_Qt-${QT_VERSION}")
else()
    vsi_configure_qt4()
endif()


# Set all 3rd party variables relative to 3rd directory
set(OSG_DIR "${THIRDPARTY_LIBRARY_DIR}/OpenSceneGraph/${OSG_VERSION}" CACHE PATH "OpenSceneGraph root directory")
set(OSGEARTH_DIR "${THIRDPARTY_LIBRARY_DIR}/OSGEarth/${OSGEARTH_VERSION}" CACHE PATH "osgEarth root directory")
set(PROTOBUF_DIR "${THIRDPARTY_LIBRARY_DIR}/protobuf/${PROTOBUF_VERSION}" CACHE PATH "protobuf root directory")
set(SQLITE3_DIR "${THIRDPARTY_LIBRARY_DIR}/SQLite/${SQLITE_VERSION}" CACHE PATH "SQLite root directory")
set(SQLITE3_LIB_NAME "sqlite-3.8")
set(GDAL_DIR "${THIRDPARTY_LIBRARY_DIR}/GDAL/${GDAL_VERSION}" CACHE PATH "3rd party library directory")

# Build Release mode by default on Linux, unless otherwise specified
if(NOT MSVC AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.")
endif()

# Force all VSI builds to have both unit tests and CDash projects
if(NOT ENABLE_CDASH_PROJECTS)
    set(ENABLE_UNIT_TESTING ON CACHE BOOL "Enable unit testing" FORCE)
    set(ENABLE_CDASH_PROJECTS ON CACHE BOOL "Generate the CDash test projects" FORCE)
    INCLUDE(CTest)
endif()

mark_as_advanced(
    OSGEARTH_DIR
    OSG_DIR
    OSG_VERSION_EXE
    PROTOBUF_DIR
    QT_QMAKE_EXECUTABLE
    SQLITE3_DIR
)
