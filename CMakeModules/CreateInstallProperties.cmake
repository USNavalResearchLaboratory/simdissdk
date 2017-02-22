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

# Select bin and lib directories by platform
set(RUNTIME_DIR "bin")
set(LIBRARY_DIR "lib")

set(INSTALLSETTINGS_RUNTIME_DIR "${RUNTIME_DIR}" CACHE STRING "Directory containing exectuables and DLLs; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
set(INSTALLSETTINGS_LIBRARY_DIR "${LIBRARY_DIR}" CACHE STRING "Directory containing shared object files (UNIX only); non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
if(WIN32)
    set(INSTALLSETTINGS_SHARED_LIBRARY_DIR "${INSTALLSETTINGS_RUNTIME_DIR}")
    set(OSGPLUGIN_DIR "${RUNTIME_DIR}/${OSG_PLUGINS}")
else()
    set(INSTALLSETTINGS_SHARED_LIBRARY_DIR "${INSTALLSETTINGS_LIBRARY_DIR}")
    set(OSGPLUGIN_DIR "${LIBRARY_DIR}/${OSG_PLUGINS}")
endif()
set(INSTALLSETTINGS_OSGPLUGIN_DIR "${OSGPLUGIN_DIR}" CACHE STRING "Directory containing plug-ins; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
# Clear temporary values
unset(RUNTIME_DIR)
unset(LIBRARY_DIR)
