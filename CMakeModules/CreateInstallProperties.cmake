# A set of functions for setting installation properties for executables, libraries,
# imported libraries, and plugins.  Only executables and plug-ins are subject to
# RPATH settings.

# Function for setting installation properties of executables
# Usage: create_executable_install_properties(TARGET COMPONENT [RPATH])
# RPATH calculates the installed location of libraries relative to the installed location of runtime objects

# Function for setting installation properties of libraries
# Usage: create_library_install_properties(TARGET COMPONENT CONFIGURATION)
# CONFIGURATION specifies Debug or Release

# Function for setting installation properties of plugins
# Usage: create_plugin_install_properties(TARGET COMPONENT CONFIGURATION)
# CONFIGURATION specifies Debug or Release

# Function for setting installation properties of imported libraries
# Usage: create_imported_library_install_properties(TARGET COMPONENT CONFIGURATION)
# CONFIGURATION specifies Debug or Release

include(InstallSettingsDirs)

function(create_executable_install_properties TARGET)
    # Separate ARGN into specific lists so that the INSTALLATION_COMPONENT can be removed from create_library arg list and iterated over
    set(arglists "INSTALLATION_COMPONENT")
    # Option names
    set(options "RPATH")

    # Get the argument lists
    parse_arguments(ARG "${arglists}" "${options}" ${ARGN})

    if(ARG_RPATH)
        # Compute RPATH for lib directory containing dependencies relative to bin installation directory
        set(LIBRARY_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
        set(RUNTIME_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_RUNTIME_DIR})
        file(RELATIVE_PATH REL_INSTALL_PATH ${RUNTIME_INSTALL_PATH} ${LIBRARY_INSTALL_PATH})

        # If the lib and bin paths were the same, an empty value will be returned
        if(NOT REL_INSTALL_PATH)
            set(REL_INSTALL_PATH ".")
        endif()

        # Set install RPATH variable
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/${REL_INSTALL_PATH}")
    endif()

    # Setup installation
    foreach(COMPONENT ${ARG_INSTALLATION_COMPONENT})
        install(TARGETS ${TARGET}
            COMPONENT ${COMPONENT}
            RUNTIME DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR})
    endforeach()
endfunction()

function(create_library_install_properties TARGET)
    # Separate ARGN into specific lists so that the INSTALLATION_COMPONENT can be removed from create_library arg list and iterated over
    set(arglists "INSTALLATION_COMPONENT")
    # Get the argument lists
    parse_arguments(ARG "${arglists}" "" ${ARGN})

    # Fall back to the non-argument version
    if(NOT ARG_INSTALLATION_COMPONENT)
        set(ARG_INSTALLATION_COMPONENT ${ARGN})
    endif()

    # Setup installation
    foreach(COMPONENT ${ARG_INSTALLATION_COMPONENT})
        install(TARGETS ${TARGET}
            COMPONENT ${COMPONENT}
            RUNTIME DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}
            LIBRARY DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR}
            ARCHIVE DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR})
    endforeach()
endfunction()


function(create_plugin_install_properties TARGET)

    # Separate ARGN into specific lists so that the INSTALLATION_COMPONENT can be removed from create_library arg list and iterated over
    set(arglists "INSTALLATION_COMPONENT")
    # Option names
    set(options "RPATH")

    # Get the argument lists
    parse_arguments(ARG "${arglists}" "${options}" ${ARGN})

    if(ARG_RPATH)
        # Compute RPATH for lib directory containing dependencies relative to bin installation directory
        set(LIBRARY_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
        set(RUNTIME_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_PLUGIN_DIR})
        file(RELATIVE_PATH REL_INSTALL_PATH ${RUNTIME_INSTALL_PATH} ${LIBRARY_INSTALL_PATH})

        # If the lib and bin paths were the same, an empty value will be returned
        if(NOT REL_INSTALL_PATH)
            set(REL_INSTALL_PATH ".")
        endif()

        # Set install RPATH variable
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/${REL_INSTALL_PATH}")
    endif()

    # Setup installation
    foreach(COMPONENT ${ARG_INSTALLATION_COMPONENT})
        install(TARGETS ${TARGET}
            COMPONENT ${COMPONENT}
            RUNTIME DESTINATION ${INSTALLSETTINGS_PLUGIN_DIR}            # Windows DLL destination
            LIBRARY DESTINATION ${INSTALLSETTINGS_PLUGIN_DIR})           # UNIX shared object destination
    endforeach()
endfunction()


# Include the code for post_build_install() command
include(PostBuildInstall)

# Defines the installation properties for a SIMDIS/Plot extension.  First it does a post-build similar to the SDK's
# post-build, creating a directory and copying the DLL/SO into it.  Next it sets the installation properties.
# Each extension should call this function after creating the target.
function(create_extension_install_properties TARGET)
    post_build_install(${TARGET} ${INSTALLSETTINGS_EXTENSION_DIR})

    # "Borrow" the routine for plug-in installation, except change the output directory.  This
    # sets installation values and updates RPATH appropriately.
    set(OLD_PLUGIN_DIR ${INSTALLSETTINGS_PLUGIN_DIR})
    set(INSTALLSETTINGS_PLUGIN_DIR ${INSTALLSETTINGS_EXTENSION_DIR})
    create_plugin_install_properties(${TARGET} RPATH INSTALLATION_COMPONENT ${ARGN})
    set(INSTALLSETTINGS_PLUGIN_DIR ${OLD_PLUGIN_DIR})
endfunction()
