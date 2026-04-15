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

# Detect configuration to use for release installs
set(_CONFIG "Release" "RelWithDebInfo" "MinSizeRel" "")
if(UNIX AND NOT EXISTS "${_OSGEARTH_LIB_DIR}/libosgEarthd.so")
    set(_CONFIG)
endif()

# Determine which files to install based on Debug or Release Build Type
if(WIN32)
    set(_DEBUG_INSTALL_PATTERN ".+d\\.dll")
    set(_RELEASE_INSTALL_PATTERN ".+[^d]\\.dll")
else()
    set(_DEBUG_INSTALL_PATTERN ".+d\\.so.*")
    set(_RELEASE_INSTALL_PATTERN ".+[^d]\\.so.*")
endif()

# Get a list of all the files in the osgEarth library directory
file(GLOB _ALL_OSGEARTH_LIBS
    LIST_DIRECTORIES false
    "${_OSGEARTH_LIB_DIR}/*.so*"
    "${_OSGEARTH_LIB_DIR}/*.dll"
)
# Categorize them into release and debug files for install
foreach(_LIB_FILE IN LISTS _ALL_OSGEARTH_LIBS)
    get_filename_component(_FILENAME ${_LIB_FILE} NAME)
    if(_FILENAME MATCHES "${_DEBUG_INSTALL_PATTERN}")
        install(FILES "${_LIB_FILE}"
            DESTINATION ${PROJECT_INSTALL_SHARED_DIR}
            CONFIGURATIONS "Debug"
            COMPONENT ThirdPartyLibs
        )
    elseif(_FILENAME MATCHES "${_RELEASE_INSTALL_PATTERN}")
        install(FILES "${_LIB_FILE}"
            DESTINATION ${PROJECT_INSTALL_SHARED_DIR}
            CONFIGURATIONS ${_CONFIG}
            COMPONENT ThirdPartyLibs
        )
    endif()
endforeach()
# Clean up variables for namespace pollution
unset(_ALL_OSGEARTH_LIBS)
unset(_LIB_FILE)
unset(_FILENAME)

# Use install(DIRECTORY to get the osgPlugins files
install(DIRECTORY ${_OSGEARTH_PLUGIN_DIR}/
    DESTINATION ${PROJECT_INSTALL_OSGPLUGINS_DIR}/osgPlugins-${OPENSCENEGRAPH_VERSION}
    CONFIGURATIONS "Debug"
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX "osgdb_${_DEBUG_INSTALL_PATTERN}"
)
install(DIRECTORY ${_OSGEARTH_PLUGIN_DIR}/
    DESTINATION ${PROJECT_INSTALL_OSGPLUGINS_DIR}/osgPlugins-${OPENSCENEGRAPH_VERSION}
    CONFIGURATIONS ${_CONFIG}
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX "osgdb_${_RELEASE_INSTALL_PATTERN}"
)
unset(_CONFIG)
