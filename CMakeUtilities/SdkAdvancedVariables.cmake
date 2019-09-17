# This file is currently include()'d by top level CMakeLists.cmake

mark_as_advanced(

    # Most users won't need to modify these settings
    CMAKE_DEBUG_POSTFIX
    CMAKE_CONFIGURATION_TYPES
    COMPILE_32_ON_64

    # Most users don't have access to internal CDash
    ENABLE_CDASH_PROJECTS
    ENABLE_FOLDERS
    BUILD_TESTING

    # Most users don't need this
    ENABLE_STATIC_ANALYSIS
    ENABLE_COVERAGE
    COVERAGE_COMMAND
    MEMORYCHECK_COMMAND
    MEMORYCHECK_SUPPRESSIONS_FILE

    # GDAL is only a dependency when using PublicDefaults
    GDAL_DIR

    # Shouldn't change in most circumstances
    INSTALLSETTINGS_LIBRARY_DIR
    INSTALLSETTINGS_OSGPLUGIN_DIR
    INSTALLSETTINGS_RUNTIME_DIR

    # Auto-detected OSG values
    APPEND_OPENSCENEGRAPH_VERSION
    OSGEARTH_PLUGIN_PATH
    
    # Qt Paths
    Qt5Core_DIR
    Qt5Gui_DIR
    Qt5Xml_DIR
    Qt5Widgets_DIR
    Qt5OpenGL_DIR
)
