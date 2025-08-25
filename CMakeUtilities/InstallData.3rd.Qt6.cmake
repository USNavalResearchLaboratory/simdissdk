# Return early if Qt6 isn't found
if(NOT TARGET Qt6::Core)
    return()
endif()

# sets up installation preferences for LIBNAME, i.e. Core; uses ARGN for component list
function(vsi_install_qt6_lib LIBNAME)
    # Ui Plug-in has nothing to install
    if(${LIBNAME} STREQUAL "UiPlugin")
        return()
    endif()

    if(WIN32)
        if(NOT TARGET Qt6::${LIBNAME})
            return()
        endif()
        get_target_property(RELEASE_DLL Qt6::${LIBNAME} LOCATION_Release)
        get_target_property(DEBUG_DLL Qt6::${LIBNAME} LOCATION_Debug)
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
        set(_QT_LIBRARY_DIR "${Qt6Core_DIR}/../..")
        INSTALL(FILES ${_QT_LIBRARY_DIR}/libQt6${LIBNAME}.so.${Qt6Core_VERSION_MAJOR}
            DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
            CONFIGURATIONS Release
            COMPONENT ThirdPartyLibs
            OPTIONAL
        )
        INSTALL(FILES ${_QT_LIBRARY_DIR}/libQt6${LIBNAME}.so.${Qt6Core_VERSION}
            DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
            CONFIGURATIONS Release
            COMPONENT ThirdPartyLibs
            OPTIONAL
        )
    endif()
endfunction()

# Macro for installing platform, media, image, xcbglintegrations plug-ins for Qt
function(vsi_install_qt6plugins dir)
    if(NOT EXISTS "${QT6_INSTALL_PREFIX}/plugins/${dir}")
        return()
    endif()
    if(WIN32)
        INSTALL(DIRECTORY ${QT6_INSTALL_PREFIX}/plugins/${dir}
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}/
            OPTIONAL
            COMPONENT ThirdPartyLibs
            CONFIGURATIONS Release RelWithDebInfo
            FILES_MATCHING PATTERN *.dll
            PATTERN *d.dll EXCLUDE)
        INSTALL(DIRECTORY ${QT6_INSTALL_PREFIX}/plugins/${dir}
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}/
            OPTIONAL
            COMPONENT ThirdPartyLibs
            CONFIGURATIONS Debug
            FILES_MATCHING PATTERN *d.dll)
    else()
        # Note that Qt requires the Linux shared objects in the executable's subdirectory (e.g. bin)
        INSTALL(DIRECTORY ${QT6_INSTALL_PREFIX}/plugins/${dir}
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}/
            OPTIONAL
            COMPONENT ThirdPartyLibs
            FILES_MATCHING PATTERN *.so)
    endif()
endfunction()

if(DEFINED INSTALL_THIRDPARTY_LIBRARIES AND NOT INSTALL_THIRDPARTY_LIBRARIES)
    return()
endif()

# Install common DLLs, if the appropriate target is found
foreach(PACKAGENAME IN ITEMS Core DBus Gui OpenGL OpenGLWidgets Widgets XcbQpa)
    vsi_install_qt6_lib(${PACKAGENAME})
endforeach()

# Each install needs some defaults
foreach(PLUGINNAME IN ITEMS imageformats platforms styles xcbglintegrations)
    vsi_install_qt6plugins(${PLUGINNAME})
endforeach()

# Users can add custom modules to install using QT6_MODULES variable
if(DEFINED QT6_MODULES)
    foreach(PACKAGENAME IN LISTS QT6_MODULES)
        vsi_install_qt6_lib(${PACKAGENAME})
    endforeach()
endif()

# Users can add custom modules to install using the QT6_PLUGINS variable
if(DEFINED QT6_PLUGINS)
    foreach(PLUGINNAME IN LISTS QT6_PLUGINS)
        vsi_install_qt6plugins(${PLUGINNAME})
    endforeach()
endif()
