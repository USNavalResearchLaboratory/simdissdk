# Expect that find_package(OpenSceneGraph) was already called in scope
if(NOT OPENSCENEGRAPH_FOUND)
    return()
endif()

# Find OSG Plug-ins for installing
find_path(OSG_PLUGIN_PATH
    NAMES osgdb_osg.dll osgdb_osg.so
    PATH_SUFFIXES
        bin/osgPlugins-${OPENSCENEGRAPH_VERSION}
        lib/osgPlugins-${OPENSCENEGRAPH_VERSION}
        lib64/osgPlugins-${OPENSCENEGRAPH_VERSION}
        bin/osgPlugins
        lib/osgPlugins
        lib64/osgPlugins
)
if(NOT OSG_PLUGIN_PATH)
    return()
endif()
get_filename_component(_OSG_LIB_DIR "${OSG_PLUGIN_PATH}" DIRECTORY)

# Determine which files to install based on Debug or Release Build Type
if(WIN32)
    set(_DEBUG_INSTALL_PATTERN ".+(_d|d)\\.dll")
    set(_RELEASE_INSTALL_PATTERN ".*[^d]\\.dll")
else()
    set(_DEBUG_INSTALL_PATTERN ".+(_d|d)\\.so.*")
    set(_RELEASE_INSTALL_PATTERN ".*[^d]\\.so.*")
endif()

install(DIRECTORY ${_OSG_LIB_DIR}/
    DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
    CONFIGURATIONS "Debug"
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX ${_DEBUG_INSTALL_PATTERN}
        PATTERN "pkgconfig" EXCLUDE
)

# osg_p3d.dll/so is release, and need special treatment
install(DIRECTORY ${OSG_PLUGIN_PATH}
    DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}/
    CONFIGURATIONS "Debug"
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX "osgdb_${_DEBUG_INSTALL_PATTERN}"
        PATTERN "*p3d.*" EXCLUDE
)

# SIM-13848: Install release DLLs on Linux if debug not found
set(_CONFIG "Release" "RelWithDebInfo" "MinSizeRel" "")
if(UNIX AND NOT EXISTS "${OSG_PLUGIN_PATH}/osgdb_rotd.so")
    set(_CONFIG)
endif()

install(DIRECTORY ${_OSG_LIB_DIR}/
    DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
    CONFIGURATIONS ${_CONFIG}
    COMPONENT ThirdPartyLibs
    FILES_MATCHING
        REGEX ${_RELEASE_INSTALL_PATTERN}
        PATTERN "pkgconfig" EXCLUDE
)

install(DIRECTORY ${OSG_PLUGIN_PATH}
    DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
    CONFIGURATIONS ${_CONFIG}
    COMPONENT ThirdPartyLibs
        REGEX "osgdb_${_RELEASE_INSTALL_PATTERN}"
        PATTERN "*p3d.*"
)
