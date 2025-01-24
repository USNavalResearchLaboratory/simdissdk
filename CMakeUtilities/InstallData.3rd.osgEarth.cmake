if(NOT TARGET osgEarth::osgEarth)
    return()
endif()

# Install if INSTALL_THIRDPARTY_LIBRARIES is undefined, or if it is set to true
if(DEFINED INSTALL_THIRDPARTY_LIBRARIES AND NOT INSTALL_THIRDPARTY_LIBRARIES)
    return()
endif()

if(TARGET osgEarth::osgEarthSilverLining)
    set(HAVE_OSGEARTH_SILVERLINING TRUE)
endif()
if(TARGET osgEarth::osgEarthTriton)
    set(HAVE_OSGEARTH_TRITON TRUE)
endif()


vsi_set_imported_locations_from_implibs(OSGEARTH)
if(TARGET osgEarth::osgEarthSilverLining)
    vsi_set_imported_locations_from_implibs(OSGEARTH_SILVERLINING)
endif()
if(TARGET osgEarth::osgEarthTriton)
    vsi_set_imported_locations_from_implibs(OSGEARTH_TRITON)
endif()

vsi_install_target(osgEarth::osgEarth ThirdPartyLibs)
if(TARGET osgEarth::osgEarthSilverLining)
    vsi_install_target(osgEarth::osgEarthSilverLining ThirdPartyLibs)
endif()
if(TARGET osgEarth::osgEarthTriton)
    vsi_install_target(osgEarth::osgEarthTriton ThirdPartyLibs)
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

# Set the full install path to the plugin directory
if(WIN32)
    set(_OSGEARTH_PLUGIN_INSTALL_DIR ${INSTALLSETTINGS_RUNTIME_DIR})
else()
    set(_OSGEARTH_PLUGIN_INSTALL_DIR ${INSTALLSETTINGS_LIBRARY_DIR})
endif()
set(_OSGEARTH_PLUGIN_INSTALL_DIR ${_OSGEARTH_PLUGIN_INSTALL_DIR}/osgPlugins-${OpenSceneGraph_VERSION})

# Determine which files to install based on Debug or Release Build Type
if(WIN32)
    set(_DEBUG_INSTALL_PATTERN "osgdb_.+d\\.dll")
    set(_RELEASE_INSTALL_PATTERN "osgdb_.+[^d]\\.dll")
else()
    set(_DEBUG_INSTALL_PATTERN "osgdb_.+d\\.so")
    set(_RELEASE_INSTALL_PATTERN "osgdb_.+[^d]\\.so")
endif()

# Note that "*billboard.*" is a release pattern, not debug
install(DIRECTORY ${_OSGEARTH_PLUGIN_DIR}/
    DESTINATION ${_OSGEARTH_PLUGIN_INSTALL_DIR}
    CONFIGURATIONS "Debug"
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX ${_DEBUG_INSTALL_PATTERN}
        PATTERN "*billboard.*" EXCLUDE)

# Note that "*billboard.*" is a release pattern and needs inclusion
install(DIRECTORY ${_OSGEARTH_PLUGIN_DIR}/
    DESTINATION ${_OSGEARTH_PLUGIN_INSTALL_DIR}
    CONFIGURATIONS "Release" "RelWithDebInfo" "MinSizeRel" ""
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX ${_RELEASE_INSTALL_PATTERN}
        PATTERN "*billboard.*")
