# Setup osgEarth library
# Setting the OSGEARTH_DIR environment variable will allow use of a custom built library

# Cannot locate osgEarth without OSG due to requirement on knowing OSG version
if(NOT OSG_FOUND)
    return()
endif()

set(LIBRARYNAME OSGEARTH)
set(VSI_OSGEARTH_VERSION OSG-${OSG_VERSION})

# Setup search paths
initialize_ENV(${LIBRARYNAME}_DIR)
set(INCLUDE_DIRS
    $ENV{${LIBRARYNAME}_DIR}/include
    ${THIRD_DIR}/osgEarth/${VSI_OSGEARTH_VERSION}/include
)
set(LIB_DIRS
    $ENV{${LIBRARYNAME}_DIR}
    ${THIRD_DIR}/osgEarth/${VSI_OSGEARTH_VERSION}
)

# Configure the core osgEarth library
find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH NAME osgEarth/Version PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES osgEarthd PATHS ${LIB_DIRS} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES osgEarth PATHS ${LIB_DIRS} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)

# MSVC requires -Zc:__cplusplus due to weejobs.h testing the value in header
if(MSVC)
    list(APPEND ${LIBRARYNAME}_COMPILE_OPTIONS "-Zc:__cplusplus")
endif()

if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    mark_as_advanced(CLEAR ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    return()
endif()
mark_as_advanced(FORCE ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)

# Configure compile and link flags for osgEarth
if(TARGET GDAL::GDAL)
    list(APPEND ${LIBRARYNAME}_LINK_LIBRARIES GDAL::GDAL)
endif()
if(TARGET GEOS::geos_c)
    list(APPEND ${LIBRARYNAME}_LINK_LIBRARIES GEOS::geos_c)
endif()
if(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthSilverLining/SilverLiningOptions")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_SILVERLINING)
    set(HAVE_OSGEARTH_SILVERLINING TRUE)
    list(APPEND SUBLIBRARY_NAMES SilverLining)
endif()

if(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthTriton/TritonLayer")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_TRITON)
    set(HAVE_OSGEARTH_TRITON TRUE)
    list(APPEND SUBLIBRARY_NAMES Triton)
elseif(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthTriton/TritonOptions")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_TRITON)
    set(HAVE_OSGEARTH_TRITON TRUE)
    list(APPEND SUBLIBRARY_NAMES Triton)
endif()

# Internal macro to import osgEarth libraries.  NAME can be empty string, Util, Symbology, etc.
macro(import_osgearth_lib NAMEVAL)
    if("${NAMEVAL}" STREQUAL "")
        set(LIBRARYNAME OSGEARTH)
    else()
        string(TOUPPER OSGEARTH_${NAMEVAL} LIBRARYNAME)
    endif()
    find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES osgEarth${NAMEVAL}d PATHS ${LIB_DIRS} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
    find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES osgEarth${NAMEVAL} PATHS ${LIB_DIRS} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
    list(APPEND ${LIBRARYNAME}_LINK_LIBRARIES OSGVIEWER OSGGA OPENTHREADS)

    # Determine whether we found the library correctly
    if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
        set(${LIBRARYNAME}_FOUND FALSE)
        mark_as_advanced(CLEAR ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
        break()
    endif()
    mark_as_advanced(FORCE ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    set(${LIBRARYNAME}_FOUND TRUE)

    # Set the release path, include path, and link libraries
    add_library(${LIBRARYNAME} SHARED IMPORTED)
    set_target_properties(${LIBRARYNAME} PROPERTIES
        IMPORTED_IMPLIB "${${LIBRARYNAME}_LIBRARY_RELEASE_NAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}"
        INTERFACE_COMPILE_OPTIONS "${${LIBRARYNAME}_COMPILE_OPTIONS}"
        INTERFACE_COMPILE_DEFINITIONS "${${LIBRARYNAME}_COMPILE_DEFINITIONS}"
        INTERFACE_LINK_LIBRARIES "${${LIBRARYNAME}_LINK_LIBRARIES}"
    )
    if(${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
        set_target_properties(${LIBRARYNAME} PROPERTIES
            IMPORTED_IMPLIB_DEBUG "${${LIBRARYNAME}_LIBRARY_DEBUG_NAME}"
        )
    endif()
endmacro()

import_osgearth_lib("")
foreach(SUBNAME ${SUBLIBRARY_NAMES})
    import_osgearth_lib(${SUBNAME})
endforeach()

# Create "normalized" targets based on osgEarth CMake install
add_library(osgEarth::osgEarth ALIAS OSGEARTH)
set(osgEarth_INCLUDE_DIR "${OSGEARTH_LIBRARY_INCLUDE_PATH}")
set(osgEarth_FOUND TRUE)
if(HAVE_OSGEARTH_SILVERLINING)
    add_library(osgEarth::osgEarthSilverLining ALIAS OSGEARTH_SILVERLINING)
endif()
if(HAVE_OSGEARTH_TRITON)
    add_library(osgEarth::osgEarthTriton ALIAS OSGEARTH_TRITON)
endif()
