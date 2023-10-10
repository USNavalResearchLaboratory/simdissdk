#.rst:
# InstallSettingsDirs
# -------------------
#
# Define VSI-based standard installation directories.
#
# Provides install directory variables as defined by the requirements of the
# default SIMDIS installation paths in the SIMDIS build tree.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# Inclusion of this module defines the following cache variables:
#
# ``INSTALLSETTINGS_RUNTIME_DIR``
#
# Destination for executables including DLLs on Windows systems.  The path is relative to the
# ``CMAKE_INSTALL_PREFIX``.
#
# ``INSTALLSETTINGS_LIBRARY_DIR``
#
# Destination for libraries including SOs on Linux systems.  The path is relative to the
# ``CMAKE_INSTALL_PREFIX``.
#
# ``INSTALLSETTINGS_PLUGIN_DIR``
#
# Destination specifically for SIMDIS Plug-ins, identified as shared objects built with the
# ``IS_PLUGIN`` target property set to ``1``.  The path is relative to the ``CMAKE_INSTALL_PREFIX``.
#
# ``INSTALLSETTINGS_CMAKE_DIR``
#
# Destination for CMake configuration modules for installed libraries.  The path is relative to
# the ``CMAKE_INSTALL_PREFIX``.
#
# ``INSTALLSETTINGS_INCLUDE_DIR``
#
# Destination for include files for installed libraries.  The path is relative to the
# ``CMAKE_INSTALL_PREFIX``.
#
# The module also defines the following non-cache helper variables:
#
# ``INSTALLSETTINGS_SHARED_LIBRARY_DIR``
#
# Destination for shared libraries.  On Windows systems, this is the same as the
# ``INSTALLSETTINGS_RUNTIME_DIR`` where dynamic link libraries (DLLs) are placed.  On Linux systems,
# this is the same as ``INSTALLSETTINGS_LIBRARY_DIR`` where shared objects (SOs) are placed.
#
# ``INSTALLSETTINGS_EXTENSION_DIR``
#
# Destination specifically for SIMDIS extensions.  The path is relative under the
# ``INSTALLSETTINGS_SHARED_LIBRARY_DIR`` path.

# Form the HW-OS (amd64-nt, amd64-linux, etc)
set(_HW amd64)
if(WIN32)
    set(_OS nt)
else()
    set(_OS linux)
endif()

# Select bin and lib directories by platform
set(INSTALLSETTINGS_RUNTIME_DIR "bin/${_HW}-${_OS}" CACHE STRING "Directory containing exectuables and DLLs; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
set(INSTALLSETTINGS_LIBRARY_DIR "lib/${_HW}-${_OS}" CACHE STRING "Directory containing shared object files (UNIX only); non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
set(INSTALLSETTINGS_PLUGIN_DIR "plugins" CACHE STRING "Directory containing plug-ins; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
set(INSTALLSETTINGS_CMAKE_DIR "lib/cmake" CACHE STRING "Directory for CMake configuration modules; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")
set(INSTALLSETTINGS_INCLUDE_DIR "include" CACHE STRING "Directory for include files; non-absolute paths are relative to CMAKE_INSTALL_PREFIX")

# Clear internal variables
unset(_HW)
unset(_OS)

# Cache-based variables are advanced
mark_as_advanced(
    INSTALLSETTINGS_RUNTIME_DIR
    INSTALLSETTINGS_LIBRARY_DIR
    INSTALLSETTINGS_PLUGIN_DIR
    INSTALLSETTINGS_CMAKE_DIR
    INSTALLSETTINGS_INCLUDE_DIR
)

# Define the shared library dir, but do not overwrite existing values
if(NOT INSTALLSETTINGS_SHARED_LIBRARY_DIR)
    if(WIN32)
        set(INSTALLSETTINGS_SHARED_LIBRARY_DIR "${INSTALLSETTINGS_RUNTIME_DIR}")
    else()
        set(INSTALLSETTINGS_SHARED_LIBRARY_DIR "${INSTALLSETTINGS_LIBRARY_DIR}")
    endif()
endif()
if(NOT INSTALLSETTINGS_EXTENSION_DIR)
    set(INSTALLSETTINGS_EXTENSION_DIR "${INSTALLSETTINGS_SHARED_LIBRARY_DIR}/extensions")
endif()
