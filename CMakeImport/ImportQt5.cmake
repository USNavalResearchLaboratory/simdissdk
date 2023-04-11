# Set up Qt5 library.  Core, Gui, and Widgets are found automatically, as well as XcbQpa and DBus on Linux.
# Extra modules can be located by setting the QT5_MODULES variable.  If unset, QT5_MODULES includes a wide
# variety of modules used by the SIMDIS software.  It can be useful to limit what is found here to keep
# your CMake variables cleaner for end users.
#
# The QT5_PLUGINS variable is used in a similar way to determine which Qt5 plugins directories to copy
# for install.  The platforms/ and imageformats/ directories are copied in automatically.  Extra plugins
# to copy in can be added by setting the QT5_PLUGINS variable.  If unset, QT5_PLUGINS includes a wide
# variety of plugins used by the SIMDIS software.

# Initialize QT5_MODULES
if(NOT DEFINED QT5_MODULES)
    set(QT5_MODULES Concurrent Designer Network OpenGL Multimedia MultimediaWidgets Svg Test UiPlugin Xml)
endif()
# Initialize QT5_PLUGINS
if(NOT DEFINED QT5_PLUGINS)
    set(QT5_PLUGINS mediaservice audio)
    if(NOT WIN32)
        list(APPEND QT5_PLUGINS xcbglintegrations)
    endif()
endif()

# Figure out the expected version and expected folder based on VSI layout,
# so that we can configure a good guess at the CMAKE_PREFIX_PATH required.
# If your Qt is somewhere else, configure CMAKE_PREFIX_PATH appropriately.
set(EXPECTED_QT5_VERSION 5.9.8)

# VSI installs to a win64_vc-#.# subdirectory under c:/QtSDK.  By default, Qt is in /usr/local/Qt-* on both systems
set(DEFAULT_QT_LOCATION "c:/QtSDK/${BUILD_SYSTEM_CANONICAL_NAME}/${EXPECTED_QT5_VERSION}")
if(NOT WIN32 OR NOT EXISTS "${DEFAULT_QT_LOCATION}")
    set(DEFAULT_QT_LOCATION "/usr/local/Qt-${EXPECTED_QT5_VERSION}")
endif()
# Append to CMAKE_PREFIX_PATH so that we can use find_package()
if(EXISTS "${DEFAULT_QT_LOCATION}")
    list(APPEND CMAKE_PREFIX_PATH "${DEFAULT_QT_LOCATION}")
endif()

# Find Qt5 Widgets
find_package(Qt5Widgets QUIET)
if(NOT Qt5Widgets_FOUND)
    return()
endif()

# SDK-146: Update EXPECTED_QT_VERSION based off the actual version string from Qt that was specified
set(EXPECTED_QT5_VERSION "${Qt5Widgets_VERSION_STRING}")
mark_as_advanced(Qt5Widgets_DIR)
mark_as_advanced(Qt5Core_DIR)
mark_as_advanced(Qt5Gui_DIR)

# Set up the version numbers
set(QT_VERSION ${Qt5Widgets_VERSION_STRING})
string(REPLACE "." ";" QT_VERSION_LIST ${QT_VERSION})
list(GET QT_VERSION_LIST 0 QT_VERSION_MAJOR)
list(GET QT_VERSION_LIST 1 QT_VERSION_MINOR)
list(GET QT_VERSION_LIST 2 QT_VERSION_PATCH)
if(VERBOSE)
    message(STATUS "Found Qt5 version: ${QT_VERSION}")
endif()

# sets up installation preferences for LIBNAME, i.e. Core; uses ARGN for component list
macro(INSTALL_QT5_LIB LIBNAME)
    if(WIN32)
        get_target_property(RELEASE_DLL Qt5::${LIBNAME} LOCATION_Release)
        get_target_property(DEBUG_DLL Qt5::${LIBNAME} LOCATION_Debug)
        if(RELEASE_DLL)
            INSTALL(FILES ${RELEASE_DLL}
                DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
                CONFIGURATIONS Release
                COMPONENT ThirdPartyLibs
            )
        endif()
        if(DEBUG_DLL)
            INSTALL(FILES ${DEBUG_DLL}
                DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
                CONFIGURATIONS Debug
                COMPONENT ThirdPartyLibs
            )
        endif()
    else()
        set(QT5_LIBRARY_DIR "${Qt5Widgets_DIR}/../..")
        INSTALL(FILES ${QT5_LIBRARY_DIR}/libQt5${LIBNAME}.so.5
            DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
            CONFIGURATIONS Release
            COMPONENT ThirdPartyLibs
        )
        INSTALL(FILES ${QT5_LIBRARY_DIR}/libQt5${LIBNAME}.so.${Qt5Widgets_VERSION_STRING}
            DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
            CONFIGURATIONS Release
            COMPONENT ThirdPartyLibs
        )
    endif()
endmacro()

# Macro for installing platform, media, image, xcbglintegrations plug-ins for Qt
macro(install_qtplugins dir)
    if(WIN32)
        INSTALL(DIRECTORY ${_qt5Gui_install_prefix}/plugins/${dir}
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}/
            OPTIONAL
            COMPONENT ThirdPartyLibs
            FILES_MATCHING PATTERN *.dll
            PATTERN *d.dll EXCLUDE)
    else()
        # Note that Qt requires the Linux shared objects in the executable's subdirectory (e.g. bin)
        INSTALL(DIRECTORY ${_qt5Gui_install_prefix}/plugins/${dir}
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}/
            OPTIONAL
            COMPONENT ThirdPartyLibs
            FILES_MATCHING PATTERN *.so)
    endif()
endmacro()

# Install each of the always-on libs

if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
    INSTALL_QT5_LIB(Core)
    INSTALL_QT5_LIB(Gui)
    INSTALL_QT5_LIB(Widgets)
    if(NOT WIN32)
        INSTALL_QT5_LIB(DBus)
        INSTALL_QT5_LIB(XcbQpa)
    endif()

    # Each install needs platforms and image formats
    install_qtplugins(platforms)
    install_qtplugins(imageformats)
    install_qtplugins(styles)
endif()

# At this point, the Widgets package is found -- find the others too
set(QT_DESIGNER_PLUGIN_DIR "${_qt5Core_install_prefix}/plugins/designer")
foreach(PACKAGENAME IN LISTS QT5_MODULES)
    find_package(Qt5${PACKAGENAME} QUIET)
    if(TARGET Qt5::${PACKAGENAME})
        # Ui Plug-in has nothing to install
        if(NOT ${PACKAGENAME} STREQUAL "UiPlugin")
            if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
                INSTALL_QT5_LIB(${PACKAGENAME})
            endif()
        endif()
        mark_as_advanced(Qt5${PACKAGENAME}_DIR)
    else()
        mark_as_advanced(CLEAR Qt5${PACKAGENAME}_DIR)
    endif()
endforeach()

# Install each of the configured plugins
if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
    foreach(PLUGINNAME IN LISTS QT5_PLUGINS)
        install_qtplugins(${PLUGINNAME})
    endforeach()
endif()

set(QT_FOUND TRUE)
set(QT5_FOUND TRUE)
