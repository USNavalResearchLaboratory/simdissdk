if(TARGET GDAL::GDAL)
    # Try to get GDAL_ROOT
    get_target_property(_GDAL_SO GDAL::GDAL IMPORTED_LOCATION_RELEASE)
    if(NOT _GDAL_SO)
        get_target_property(_GDAL_SO GDAL::GDAL IMPORTED_LOCATION)
    endif()
    get_filename_component(_GDAL_LIB_DIR "${_GDAL_SO}" DIRECTORY)
    get_filename_component(_GDAL_DIR "${_GDAL_LIB_DIR}" DIRECTORY)

    if(WIN32)
        vsi_install_target(GDAL::GDAL ThirdPartyLibs)
    else()
        get_symlinks("${_GDAL_LIB_DIR}/libgdal.so" _GDAL_LIBS)
        install(PROGRAMS ${_GDAL_LIBS} DESTINATION "${INSTALLSETTINGS_SHARED_LIBRARY_DIR}" COMPONENT ThirdPartyLibs)
    endif()

    # Install GDAL plugins
    set(_PLUGIN_DIRS
        ${_GDAL_DIR}/lib
        ${_GDAL_DIR}/lib64
        ${_GDAL_DIR}/bin
    )
    find_path(INSTALLSOURCE_GDAL_PLUGINS_DIR gdalplugins ${_PLUGIN_DIRS} NO_DEFAULT_PATH)
    if(NOT INSTALLSOURCE_GDAL_PLUGINS_DIR)
        mark_as_advanced(CLEAR INSTALLSOURCE_GDAL_PLUGINS_DIR)
    else()
        mark_as_advanced(FORCE INSTALLSOURCE_GDAL_PLUGINS_DIR)
        install(DIRECTORY "${INSTALLSOURCE_GDAL_PLUGINS_DIR}/gdalplugins/"
            DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}/gdalplugins
            COMPONENT ThirdPartyLibs)
    endif()

    # SIMDIS GDAL data
    set(_DATA_DIRS
        ${_GDAL_DIR}/data/gdal
        ${_GDAL_DIR}/data
        ${_GDAL_DIR}/share/gdal
    )
    find_path(INSTALLSOURCE_GDAL_DATA_DIR gt_ellips.csv ${_DATA_DIRS} NO_DEFAULT_PATH)
    if(NOT INSTALLSOURCE_GDAL_DATA_DIR)
        mark_as_advanced(CLEAR INSTALLSOURCE_GDAL_DATA_DIR)
        return()
    endif()
    mark_as_advanced(FORCE INSTALLSOURCE_GDAL_DATA_DIR)
    install(DIRECTORY ${INSTALLSOURCE_GDAL_DATA_DIR}/
        DESTINATION data/GDAL
        COMPONENT ThirdPartyLibs)
endif()
