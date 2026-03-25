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
# Inclusion of this module defines the following non-cache variables:
#
# ``PROJECT_INSTALL_SHARED_DIR``
#
# Destination for shared libraries.  On Windows systems, this is the same as the
# ``CMAKE_INSTALL_BINDIR`` where dynamic link libraries (DLLs) are placed.  On Linux systems,
# this is the same as ``CMAKE_INSTALL_LIBDIR`` where shared objects (SOs) are placed.
#
# ``INSTALLSETTINGS_OSGPLUGINS_DIR``
#
# Destination parent for OpenSceneGraph plugins. Typically this is your
# ``PROJECT_INSTALL_SHARED_DIR``, under which an osgPlugins-x.x.x is created and plugins
# are placed. But vcpkg places this in a plugins/ directory instead.

# Install to lib instead of lib64
set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "Output directory for libraries")

# Pull in the CMAKE_INSTALL_ values
include(GNUInstallDirs)

# Define PROJECT_INSTALL_SHARED_DIR for shared libraries and DLLs
if(WIN32)
    set(PROJECT_INSTALL_SHARED_DIR "${CMAKE_INSTALL_BINDIR}")
else()
    set(PROJECT_INSTALL_SHARED_DIR "${CMAKE_INSTALL_LIBDIR}")
endif()

# Define osgPlugins directory root; typically the shared library directory (bin or lib); can be plugins under vcpkg
if(NOT INSTALLSETTINGS_OSGPLUGINS_DIR)
    if(USING_VCPKG)
        set(INSTALLSETTINGS_OSGPLUGINS_DIR "plugins")
    else()
        set(INSTALLSETTINGS_OSGPLUGINS_DIR "${PROJECT_INSTALL_SHARED_DIR}")
    endif()
endif()
