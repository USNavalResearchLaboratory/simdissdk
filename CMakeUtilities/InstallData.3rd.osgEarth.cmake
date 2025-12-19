if(NOT TARGET osgEarth::osgEarth)
    return()
endif()

get_target_property(_OSGEARTH_SO osgEarth::osgEarth IMPORTED_LOCATION_RELEASE)
if(NOT _OSGEARTH_SO)
    get_target_property(_OSGEARTH_SO osgEarth::osgEarth IMPORTED_LOCATION)
endif()
get_filename_component(_OSGEARTH_LIB_DIR "${_OSGEARTH_SO}" DIRECTORY)
get_filename_component(_OSGEARTH_DIR "${_OSGEARTH_LIB_DIR}" DIRECTORY)
file(GLOB _OSGEARTH_PLUGIN "${_OSGEARTH_DIR}/*/osgPlugins*/osgdb_earth.*")
if(NOT _OSGEARTH_PLUGIN)
    message(WARNING "osgEarth Plug-in Path not found.  Will result in improper installation.")
    return()
endif()
get_filename_component(_OSGEARTH_PLUGIN_DIR "${_OSGEARTH_PLUGIN}" DIRECTORY)

# Determine which files to install based on Debug or Release Build Type
if(WIN32)
    set(_DEBUG_INSTALL_PATTERN ".+d\\.dll")
    set(_RELEASE_INSTALL_PATTERN ".+[^d]\\.dll")
else()
    set(_DEBUG_INSTALL_PATTERN ".+d\\.so.*")
    set(_RELEASE_INSTALL_PATTERN ".+[^d]\\.so.*")
endif()

install(DIRECTORY ${_OSGEARTH_LIB_DIR}/
    DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
    CONFIGURATIONS "Debug"
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX ${_DEBUG_INSTALL_PATTERN}
)

# Note that "*billboard.*" is a release pattern, not debug
install(DIRECTORY ${_OSGEARTH_PLUGIN_DIR}/
    DESTINATION ${INSTALLSETTINGS_OSGPLUGINS_DIR}/osgPlugins-${OPENSCENEGRAPH_VERSION}
    CONFIGURATIONS "Debug"
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX "osgdb_${_DEBUG_INSTALL_PATTERN}"
        PATTERN "*billboard.*" EXCLUDE)

install(DIRECTORY ${_OSGEARTH_LIB_DIR}/
    DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
    CONFIGURATIONS "Release" "RelWithDebInfo" "MinSizeRel" ""
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX ${_RELEASE_INSTALL_PATTERN}
)

# Note that "*billboard.*" is a release pattern and needs inclusion
install(DIRECTORY ${_OSGEARTH_PLUGIN_DIR}/
    DESTINATION ${INSTALLSETTINGS_OSGPLUGINS_DIR}/osgPlugins-${OPENSCENEGRAPH_VERSION}
    CONFIGURATIONS "Release" "RelWithDebInfo" "MinSizeRel" ""
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX "osgdb_${_RELEASE_INSTALL_PATTERN}"
        PATTERN "*billboard.*")
