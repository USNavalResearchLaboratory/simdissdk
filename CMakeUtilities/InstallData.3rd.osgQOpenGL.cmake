if(NOT TARGET osgQt::osgQOpenGL-qt5)
    return()
endif()

if(WIN32)
    vsi_install_target(osgQt::osgQOpenGL-qt5 ThirdPartyLibs)
    return()
endif()

# Need the library directory to get symlinks from libosgQOpenGL-qt5.so
get_target_property(_OSGQOPENGL_SO osgQt::osgQOpenGL-qt5 IMPORTED_LOCATION_RELEASE)
if(NOT _OSGQOPENGL_SO)
    get_target_property(_OSGQOPENGL_SO osgQt::osgQOpenGL-qt5 IMPORTED_LOCATION)
endif()
get_filename_component(_OSGQOPENGL_LIB_DIR "${_OSGQOPENGL_SO}" DIRECTORY)
get_symlinks("${_OSGQOPENGL_LIB_DIR}/libosgQOpenGL-qt5.so" _OSGQOPENGL_LIBS)
install(PROGRAMS ${_OSGQOPENGL_LIBS} DESTINATION "${INSTALLSETTINGS_SHARED_LIBRARY_DIR}" OPTIONAL COMPONENT ThirdPartyLibs)
