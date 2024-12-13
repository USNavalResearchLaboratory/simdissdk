# Setup GDAL library
# Setting the GDAL_DIR environment variable will allow use of a custom built library

set(LIBRARYNAME GDAL)
set(${LIBRARYNAME}_VERSION 3.10.0)
set(${LIBRARYNAME}_INSTALL_COMPONENT ThirdPartyLibs)
if(BUILD_COMPILER STREQUAL "gcc-8.3")
    set(${LIBRARYNAME}_VERSION 3.8.2)
endif()

# Setup search paths based off GDAL_ROOT
initialize_ENV(${LIBRARYNAME}_DIR)
find_path(${LIBRARYNAME}_ROOT
    NAMES include/gdal.h
    HINTS
        $ENV{${LIBRARYNAME}_DIR}
        $ENV{${LIBRARYNAME}_ROOT}
        ${THIRD_DIR}/GDAL/${${LIBRARYNAME}_VERSION}
        ${THIRD_DIR}/GDAL/3.4.3
        ${THIRD_DIR}/GDAL
    NO_DEFAULT_PATH
)

# GDAL_LIBRARY_INCLUDE_PATH
find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH
    NAME gdal.h
    PATHS ${${LIBRARYNAME}_ROOT}/include NO_DEFAULT_PATH)
# GDAL_LIBRARY_RELEASE_PATH
find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME
    NAMES gdal gdal_i gdal304 gdal302 gdal204 gdal201 gdal111
    PATHS ${${LIBRARYNAME}_ROOT}/lib NO_DEFAULT_PATH)
# GDAL_LIBRARY_DEBUG_PATH
find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME
    NAMES gdal_d gdal304_d gdal302_d gdal204_d gdal201_d gdal111_d
    PATHS ${${LIBRARYNAME}_ROOT}/lib NO_DEFAULT_PATH)
# Fall back on release library explicitly, only on Windows
if(WIN32 AND ${LIBRARYNAME}_LIBRARY_RELEASE_NAME AND NOT ${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
    set(${LIBRARYNAME}_LIBRARY_DEBUG_NAME "${${LIBRARYNAME}_LIBRARY_RELEASE_NAME}" CACHE STRING "Path to a library" FORCE)
endif()

# Set up compile definitions and dependencies
set(${LIBRARYNAME}_COMPILE_DEFINITIONS CPL_DISABLE_DLL)
if(UNIX)
    list(APPEND ${LIBRARYNAME}_LINK_LIBRARIES dl)
    if(TARGET GEOS_C)
        list(APPEND ${LIBRARYNAME}_LINK_LIBRARIES GEOS_C)
    endif()
else()
    list(APPEND ${LIBRARYNAME}_LINK_LIBRARIES VSI::SOCKET)
endif()

############################################################
########## Imported 3rd party library boilerplate ##########
############################################################

# Determine whether we found the library correctly
if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    set(${LIBRARYNAME}_FOUND FALSE)
    mark_as_advanced(CLEAR ${LIBRARYNAME}_ROOT ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    return()
endif()

mark_as_advanced(FORCE ${LIBRARYNAME}_ROOT ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
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
# Deal with debug
if(${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
    set_target_properties(${LIBRARYNAME} PROPERTIES
        IMPORTED_IMPLIB_DEBUG "${${LIBRARYNAME}_LIBRARY_DEBUG_NAME}"
    )
endif()

# EL7 compatibility
add_library(GDAL::GDAL ALIAS ${LIBRARYNAME})

vsi_set_imported_locations_from_implibs(${LIBRARYNAME})
# No need to continue if not doing installation
if(DEFINED INSTALL_THIRDPARTY_LIBRARIES AND NOT INSTALL_THIRDPARTY_LIBRARIES)
    return()
endif()
vsi_install_target(${LIBRARYNAME} ${${LIBRARYNAME}_INSTALL_COMPONENT})

############################################################

# Install GDAL plugins
set(PLUGIN_DIRS
    ${${LIBRARYNAME}_ROOT}/lib
    ${${LIBRARYNAME}_ROOT}/lib64
    ${${LIBRARYNAME}_ROOT}/bin
)
find_path(INSTALLSOURCE_GDAL_PLUGINS_DIR gdalplugins ${PLUGIN_DIRS} NO_DEFAULT_PATH)
if(NOT INSTALLSOURCE_GDAL_PLUGINS_DIR)
    mark_as_advanced(CLEAR INSTALLSOURCE_GDAL_PLUGINS_DIR)
else()
    mark_as_advanced(FORCE INSTALLSOURCE_GDAL_PLUGINS_DIR)
    install(DIRECTORY "${INSTALLSOURCE_GDAL_PLUGINS_DIR}/gdalplugins/"
        DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}/gdalplugins
        COMPONENT ThirdPartyLibs)
endif()

# SIMDIS GDAL data
set(DATA_DIRS
    ${${LIBRARYNAME}_ROOT}/data/gdal
    ${${LIBRARYNAME}_ROOT}/data
    ${${LIBRARYNAME}_ROOT}/share/gdal
)
find_path(INSTALLSOURCE_GDAL_DATA_DIR gt_ellips.csv ${DATA_DIRS} NO_DEFAULT_PATH)
if(NOT INSTALLSOURCE_GDAL_DATA_DIR)
    mark_as_advanced(CLEAR INSTALLSOURCE_GDAL_DATA_DIR)
    return()
endif()
mark_as_advanced(FORCE INSTALLSOURCE_GDAL_DATA_DIR)
install(DIRECTORY ${INSTALLSOURCE_GDAL_DATA_DIR}/
    DESTINATION data/GDAL
    COMPONENT ThirdPartyLibs)
