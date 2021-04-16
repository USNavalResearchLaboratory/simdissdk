#
# Set up MrSID library (Raster and Lidar supported).
#
# This script will locate MrSID based on MRSID_DIR, MRSID_RASTER_DIR, and/or MRSID_LIDAR_DIR.
# The purpose is to install the DLL / SO dependencies into INSTALLSETTINGS_SHARED_LIBRARY_DIR.
#

# No need to continue if not doing installation
if(DEFINED INSTALL_THIRDPARTY_LIBRARIES AND NOT INSTALL_THIRDPARTY_LIBRARIES)
    return()
endif()

set(LIBRARYNAME MRSID)
set(${LIBRARYNAME}_VERSION 9.5.4.4709)
set(${LIBRARYNAME}_INSTALL_COMPONENT ThirdPartyLibs)

# Setup search paths
initialize_ENV(MRSID_DIR)
initialize_ENV(MRSID_RASTER_DIR)
initialize_ENV(MRSID_LIDAR_DIR)

# Find the root for MrSID (Raster)
find_path(MRSID_RASTER_DIR
    NAMES include/MrSIDImageReader.h lib/libltidsdk.so lib/lti_dsdk_9.5.dll
    HINTS
        $ENV{MRSID_DIR}
        $ENV{MRSID_RASTER_DIR}
        $ENV{MRSID_DIR}/Raster_DSDK
        ${THIRD_DIR}/MrSID_DSDK/${${LIBRARYNAME}_VERSION}/Raster_DSDK
    NO_DEFAULT_PATH
)
# Find the root for MrSID (Lidar)
find_path(MRSID_LIDAR_DIR
    NAMES include/lidar/MG4PointReader.h lib/liblti_lidar_dsdk.so lib/lti_lidar_dsdk_1.1.dll
    HINTS
        $ENV{MRSID_DIR}
        $ENV{MRSID_LIDAR_DIR}
        $ENV{MRSID_DIR}/Lidar_DSDK
        ${THIRD_DIR}/MrSID_DSDK/${${LIBRARYNAME}_VERSION}/Lidar_DSDK
    NO_DEFAULT_PATH
)

# Install the libraries into the shared library runtime location
macro(mrsid_install_library PATH NAME)
    set(MRSID_RELEASE_LIB "${${PATH}}/lib/${NAME}")
    if(EXISTS "${MRSID_RELEASE_LIB}")
        # Pull out all the share links too
        get_symlinks("${MRSID_RELEASE_LIB}" RELEASE_LIBS)
        install(PROGRAMS ${RELEASE_LIBS}
            DESTINATION "${INSTALLSETTINGS_SHARED_LIBRARY_DIR}"
            COMPONENT ${${LIBRARYNAME}_INSTALL_COMPONENT}
        )
    endif()
endmacro()

# Install each library
if(WIN32)
    mrsid_install_library(MRSID_RASTER_DIR lti_dsdk_9.5.dll)
    if(INSTALL_MRSID_TBB OR NOT DEFINED INSTALL_MRSID_TBB)
        mrsid_install_library(MRSID_RASTER_DIR tbb/tbb.dll)
    endif()
    mrsid_install_library(MRSID_LIDAR_DIR lti_lidar_dsdk_1.1.dll)
else()
    mrsid_install_library(MRSID_RASTER_DIR libltidsdk.so)
    if(INSTALL_MRSID_TBB OR NOT DEFINED INSTALL_MRSID_TBB)
        mrsid_install_library(MRSID_RASTER_DIR libtbb.so)
    endif()
    mrsid_install_library(MRSID_LIDAR_DIR liblti_lidar_dsdk.so)
endif()

# Mark variables as advanced
if(IS_DIRECTORY "${MRSID_RASTER_DIR}")
    mark_as_advanced(FORCE MRSID_RASTER_DIR)
else()
    mark_as_advanced(CLEAR MRSID_RASTER_DIR)
endif()
if(IS_DIRECTORY "${MRSID_LIDAR_DIR}")
    mark_as_advanced(FORCE MRSID_LIDAR_DIR)
else()
    mark_as_advanced(CLEAR MRSID_LIDAR_DIR)
endif()
