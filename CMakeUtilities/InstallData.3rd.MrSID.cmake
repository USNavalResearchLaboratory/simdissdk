find_path(MRSID_RASTER_DIR
    NAMES
        include/MrSIDImageReader.h
        lib/libltidsdk.so
        lib/lti_dsdk.lib
    PATH_SUFFIXES Raster_DSDK
)

find_path(MRSID_LIDAR_DIR
    NAMES
        include/lidar/MG4PointReader.h
        lib/liblti_lidar_dsdk.so
        lib/lti_lidar_dsdk.lib
    PATH_SUFFIXES Lidar_DSDK
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
    else()
        message(WARNING "Unable to find: ${MRSID_RELEASE_LIB}")
    endif()
endmacro()

# Install each library
if(MRSID_RASTER_DIR AND IS_DIRECTORY "${MRSID_RASTER_DIR}")
    if(WIN32)
        mrsid_install_library(MRSID_RASTER_DIR lti_dsdk_9.5.dll)
        if(INSTALL_MRSID_TBB OR NOT DEFINED INSTALL_MRSID_TBB)
            mrsid_install_library(MRSID_RASTER_DIR tbb.dll)
        endif()
    else()
        mrsid_install_library(MRSID_RASTER_DIR libltidsdk.so)
        if(INSTALL_MRSID_TBB OR NOT DEFINED INSTALL_MRSID_TBB)
            mrsid_install_library(MRSID_RASTER_DIR libtbb.so)
        endif()
    endif()
    mark_as_advanced(FORCE MRSID_RASTER_DIR)
else()
    mark_as_advanced(CLEAR MRSID_RASTER_DIR)
endif()

if(MRSID_LIDAR_DIR AND IS_DIRECTORY "${MRSID_LIDAR_DIR}")
    if(WIN32)
        mrsid_install_library(MRSID_LIDAR_DIR lti_lidar_dsdk_1.1.dll)
    else()
         mrsid_install_library(MRSID_LIDAR_DIR liblti_lidar_dsdk.so)
    endif()
    mark_as_advanced(FORCE MRSID_LIDAR_DIR)
else()
    mark_as_advanced(CLEAR MRSID_LIDAR_DIR)
endif()
