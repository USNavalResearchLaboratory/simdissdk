# Setup OSGEarth library
# Setting the OSGEARTH_DIR environment variable will allow use of a custom built library

# Cannot locate osgEarth without OSG due to requirement on knowing OSG version
if(NOT OSG_FOUND)
    return()
endif()

set(LIBRARYNAME OSGEARTH)
set(VSI_OSGEARTH_VERSION OSG-${OSG_VERSION})
set(SIMDIS_SDK_OSGEARTH_PATH ${THIRD_DIR}/osgEarth-SDK-1.4)
set(OSGEARTH_INSTALL_COMPONENT ThirdPartyLibs)
# Install if INSTALL_THIRDPARTY_LIBRARIES is undefined, or if it is set to true
set(OSGEARTH_SHOULD_INSTALL FALSE)
if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
    set(OSGEARTH_SHOULD_INSTALL TRUE)
endif()

# Setup search paths
initialize_ENV(${LIBRARYNAME}_DIR)
set(INCLUDE_DIRS
    $ENV{${LIBRARYNAME}_DIR}/include
    ${SIMDIS_SDK_OSGEARTH_PATH}/include
    ${THIRD_DIR}/OSGEarth/${VSI_OSGEARTH_VERSION}/include
)
set(LIB_DIRS
    $ENV{${LIBRARYNAME}_DIR}/lib
    $ENV{${LIBRARYNAME}_DIR}/lib64
    ${SIMDIS_SDK_OSGEARTH_PATH}/lib
    ${SIMDIS_SDK_OSGEARTH_PATH}/lib64
    ${THIRD_DIR}/OSGEarth/${VSI_OSGEARTH_VERSION}/lib
    ${THIRD_DIR}/OSGEarth/${VSI_OSGEARTH_VERSION}/lib64
)

# Configure the core osgEarth library
find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH NAME osgEarth/Version PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES osgEarthd PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES osgEarth PATHS ${LIB_DIRS} NO_DEFAULT_PATH)

if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    mark_as_advanced(CLEAR ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    return()
endif()
mark_as_advanced(FORCE ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)

# Configure compile and link flags for osgEarth
if(TARGET GDAL)
    set(${LIBRARYNAME}_LINK_LIBRARIES GDAL)
endif()
set(SUBLIBRARY_NAMES Annotation Features Splat Symbology Util)
if(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthDrivers/sky_silverlining/SilverLiningOptions")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_SILVERLINING)
    set(HAVE_OSGEARTH_SILVERLINING TRUE)
    set(OSGEARTH_SILVERLINING_SUPPORT PLUGIN)
elseif(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthSilverLining/SilverLiningOptions")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_SILVERLINING)
    set(HAVE_OSGEARTH_SILVERLINING TRUE)
    set(OSGEARTH_SILVERLINING_SUPPORT NODEKIT)
    list(APPEND SUBLIBRARY_NAMES SilverLining)
endif()

if(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthDrivers/ocean_triton/TritonOptions")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_TRITON)
    set(HAVE_OSGEARTH_TRITON TRUE)
    set(OSGEARTH_TRITON_SUPPORT PLUGIN)
elseif(EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgEarthTriton/TritonOptions")
    list(APPEND ${LIBRARYNAME}_COMPILE_DEFINITIONS HAVE_OSGEARTH_TRITON)
    set(HAVE_OSGEARTH_TRITON TRUE)
    set(OSGEARTH_TRITON_SUPPORT NODEKIT)
    list(APPEND SUBLIBRARY_NAMES Triton)
endif()

# Internal macro to import osgEarth libraries.  NAME can be empty string, Util, Symbology, etc.
macro(import_osgearth_lib NAMEVAL)
    if("${NAMEVAL}" STREQUAL "")
        set(LIBRARYNAME OSGEARTH)
    else()
        string(TOUPPER OSGEARTH_${NAMEVAL} LIBRARYNAME)
    endif()
    find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES osgEarth${NAMEVAL}d PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
    find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES osgEarth${NAMEVAL} PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
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
    vsi_set_imported_locations_from_implibs(${LIBRARYNAME})
    if(OSGEARTH_SHOULD_INSTALL)
        vsi_install_target(${LIBRARYNAME} ${OSGEARTH_INSTALL_COMPONENT})
    endif()
endmacro()

import_osgearth_lib("")
foreach(SUBNAME ${SUBLIBRARY_NAMES})
    import_osgearth_lib(${SUBNAME})
endforeach()

# Plug-ins are found in lib, unless on Windows
set(OS_PLUGIN_SUBDIR "lib${LIBDIRSUFFIX}")
if(WIN32)
  set(OS_PLUGIN_SUBDIR "bin")
endif(WIN32)
set(OS_PLUGIN_SUBDIR "${OS_PLUGIN_SUBDIR}/osgPlugins-${OSG_VERSION}")

# Install OSGEarth plugins
set(PLUGIN_DIRS
    $ENV{OSGEARTH_DIR}/bin/osgPlugins-${OSG_VERSION}
    $ENV{OSGEARTH_DIR}/lib/osgPlugins-${OSG_VERSION}
    $ENV{OSGEARTH_DIR}/lib64/osgPlugins-${OSG_VERSION}
    ${SIMDIS_SDK_OSGEARTH_PATH}/bin/osgPlugins-${OSG_VERSION}
    ${SIMDIS_SDK_OSGEARTH_PATH}/lib/osgPlugins-${OSG_VERSION}
    ${SIMDIS_SDK_OSGEARTH_PATH}/lib64/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OSGEarth/${VSI_OSGEARTH_VERSION}/bin/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OSGEarth/${VSI_OSGEARTH_VERSION}/lib/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OSGEarth/${VSI_OSGEARTH_VERSION}/lib64/osgPlugins-${OSG_VERSION}
)

# Find the plugin location
if(WIN32)
    find_path(OSGEARTH_PLUGIN_PATH NAMES osgdb_earth.dll PATHS ${PLUGIN_DIRS} NO_DEFAULT_PATH)
else(WIN32)
    find_path(OSGEARTH_PLUGIN_PATH NAMES osgdb_earth.so PATHS ${PLUGIN_DIRS} NO_DEFAULT_PATH)
endif(WIN32)

if(OSGEARTH_PLUGIN_PATH)
    mark_as_advanced(FORCE OSGEARTH_PLUGIN_PATH)
else()
    mark_as_advanced(CLEAR OSGEARTH_PLUGIN_PATH)
    message(WARNING "osgEarth Plug-in Path not found.  Will result in improper installation.")
endif()

# Put the plugin location in the library list for 32 to 64 but Linux conversion
# so it is properly updated when a 32/64 bit configuration change is made
list(APPEND LIBRARY_LIST "OSGEARTH_PLUGIN_PATH")

# Select installation location
if(WIN32)
    set(OSGEARTH_PLUGIN_INSTALL_DIR ${INSTALLSETTINGS_RUNTIME_DIR})
else(WIN32)
    set(OSGEARTH_PLUGIN_INSTALL_DIR ${INSTALLSETTINGS_LIBRARY_DIR})
endif(WIN32)

# Determine which files to install based on Debug or Release Build Type
if(WIN32)
    set(DEBUG_INSTALL_PATTERN "osgdb_.+d\\.dll")
    set(RELEASE_INSTALL_PATTERN "osgdb_.+[^d]\\.dll")
else(WIN32)
    set(DEBUG_INSTALL_PATTERN "osgdb_.+d\\.so")
    set(RELEASE_INSTALL_PATTERN "osgdb_.+[^d]\\.so")
endif(WIN32)

# Set the full install path to the plugin directory
set(OSGEARTH_PLUGIN_INSTALL_DIR ${OSGEARTH_PLUGIN_INSTALL_DIR}/osgPlugins-${OSG_VERSION})

if(OSGEARTH_SHOULD_INSTALL)
    # Note that "*billboard.*" is a release pattern, not debug
    install(DIRECTORY ${OSGEARTH_PLUGIN_PATH}/
        DESTINATION ${OSGEARTH_PLUGIN_INSTALL_DIR}
        CONFIGURATIONS "Debug"
        COMPONENT ${OSGEARTH_INSTALL_COMPONENT}
        FILES_MATCHING
            REGEX ${DEBUG_INSTALL_PATTERN}
            PATTERN "*billboard.*" EXCLUDE)

    # Note that "*billboard.*" is a release pattern and needs inclusion
    install(DIRECTORY ${OSGEARTH_PLUGIN_PATH}/
        DESTINATION ${OSGEARTH_PLUGIN_INSTALL_DIR}
        CONFIGURATIONS "Release"
        COMPONENT ${OSGEARTH_INSTALL_COMPONENT}
        FILES_MATCHING
            REGEX ${RELEASE_INSTALL_PATTERN}
            PATTERN "*billboard.*")
endif()

set(OSGEARTH_ALL_LIBDEPENDENCIES)
if(OSGEARTH_PLUGIN_PATH)
    set(OSGEARTH_ALL_LIBDEPENDENCIES OSGEARTH OSGEARTH_FEATURES OSGEARTH_SYMBOLOGY OSGEARTH_UTIL)
    list(APPEND OSGEARTH_ALL_LIBDEPENDENCIES OSGEARTH_ANNOTATION)
endif()
