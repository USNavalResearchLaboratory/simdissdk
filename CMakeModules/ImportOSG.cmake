# Setup OSG library
# Setting the OSG_DIR environment variable will allow use of a custom built library

set(OPENSCENEGRAPH_FOUND FALSE)
set(LIBRARYNAME OSG)

# OpenThreads is stored under the OpenSceneGraph folder
set(OSG_VERSION 3.4.0)
# Adjust for the Qt version.  Note that 4.8 version works across the board for 4.8.x
set(OSG_SUBDIR ${OSG_VERSION}_Qt-${QT_VERSION})
set(OSG_DLL_PREFIX osg130-)
set(${LIBRARYNAME}_INSTALL_COMPONENT ThirdPartyLibs)
# Install if INSTALL_THIRDPARTY_LIBRARIES is undefined, or if it is set to true
set(OSG_SHOULD_INSTALL FALSE)
if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
    set(OSG_SHOULD_INSTALL TRUE)
endif()

initialize_ENV(OSG_DIR)
set(INCLUDE_DIRS
    ${OSG_DIR}/include
    $ENV{OSG_DIR}/include
    ${THIRD_DIR}/OpenSceneGraph-${OSG_VERSION}/include
    ${THIRD_DIR}/OpenSceneGraph/${OSG_SUBDIR}/include
)

set(LIB_DIRS 
    ${OSG_DIR}/lib
    ${OSG_DIR}/lib64
    $ENV{OSG_DIR}/lib
    $ENV{OSG_DIR}/lib64
    ${THIRD_DIR}/OpenSceneGraph-${OSG_VERSION}/lib
    ${THIRD_DIR}/OpenSceneGraph-${OSG_VERSION}/lib64
    ${THIRD_DIR}/OpenSceneGraph/${OSG_SUBDIR}/lib
    ${THIRD_DIR}/OpenSceneGraph/${OSG_SUBDIR}/lib64
)

# Configure the core OSG library
find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH NAME osg/Version PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES osgd PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES osg PATHS ${LIB_DIRS} NO_DEFAULT_PATH)

# osg_add_filename_prefix(<VAR> <FILENAME> <PREFIX>)
#
# Adds a prefix <PREFIX> to the name portion of file <FILENAME>, storing the
# result in <VAR>.  For example, given add_filename_prefix(FOO "/tmp/file" "my"),
# the resulting ${FOO} will be "/tmp/myfile"
function(osg_add_filename_prefix VAR FILENAME PREFIX)
    get_filename_component(BASE_PATH ${FILENAME} DIRECTORY)
    get_filename_component(NAME_ONLY ${FILENAME} NAME_WE)
    set(FULL_PATH "${BASE_PATH}/../bin/${PREFIX}${NAME_ONLY}.dll")
    get_filename_component(FULL_PATH "${FULL_PATH}" ABSOLUTE)
    set(${VAR} ${FULL_PATH} PARENT_SCOPE)
endfunction()

# osg_set_imported_locations_from_implibs(<TARGET> <WIN_PREFIX>)
#
# Same as vsi_set_imported_locations_from_implibs(), but specialized for OSG DLLs.  On Windows,
# the OSG DLLs are prefixed with a version.  This method will set the version prefix on the
# IMPORTED_LOCATION and IMPORTED_LOCATION_DEBUG property for OSG DLLs.
function(osg_set_imported_locations_from_implibs TARGET WIN_PREFIX)
    # Call the vsi_set_imported_locations_from_implibs first to initialize IMPORTED_LOCATION values
    vsi_set_imported_locations_from_implibs(${TARGET})
    # On Windows, we add a prefix to the path
    if(NOT WIN32)
        return()
    endif()
    get_target_property(LOCATION_RELEASE ${TARGET} IMPORTED_LOCATION)
    if(LOCATION_RELEASE)
        osg_add_filename_prefix(LOCATION_RELEASE "${LOCATION_RELEASE}" ${WIN_PREFIX})
        set_target_properties(${TARGET} PROPERTIES IMPORTED_LOCATION ${LOCATION_RELEASE})
    endif()
    get_target_property(LOCATION_DEBUG ${TARGET} IMPORTED_LOCATION_DEBUG)
    if(LOCATION_DEBUG)
        osg_add_filename_prefix(LOCATION_DEBUG "${LOCATION_DEBUG}" ${WIN_PREFIX})
        set_target_properties(${TARGET} PROPERTIES IMPORTED_LOCATION_DEBUG ${LOCATION_DEBUG})
    endif()
endfunction()

############################################################
########## Imported 3rd party library boilerplate ##########
############################################################

# Determine whether we found the library correctly
if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    set(${LIBRARYNAME}_FOUND FALSE)
    mark_as_advanced(CLEAR ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME OSG_DIR)
    return()
endif()

mark_as_advanced(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME OSG_DIR)
set(${LIBRARYNAME}_FOUND TRUE)

# Set the release path, include path, and link libraries
add_library(${LIBRARYNAME} SHARED IMPORTED)
set_target_properties(${LIBRARYNAME} PROPERTIES
    IMPORTED_IMPLIB "${${LIBRARYNAME}_LIBRARY_RELEASE_NAME}"
    INTERFACE_INCLUDE_DIRECTORIES "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}"
    INTERFACE_COMPILE_OPTIONS "${${LIBRARYNAME}_COMPILE_OPTIONS}"
    INTERFACE_COMPILE_DEFINITIONS "${${LIBRARYNAME}_COMPILE_DEFINITIONS}"
    INTERFACE_LINK_LIBRARIES "${${LIBRARYNAME}_LINK_LIBRARIES}"
)
if(${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
    set_target_properties(${LIBRARYNAME} PROPERTIES
        IMPORTED_IMPLIB_DEBUG "${${LIBRARYNAME}_LIBRARY_DEBUG_NAME}"
    )
endif()
osg_set_imported_locations_from_implibs(${LIBRARYNAME} ${OSG_DLL_PREFIX})
if(OSG_SHOULD_INSTALL)
    vsi_install_target(${LIBRARYNAME} ${${LIBRARYNAME}_INSTALL_COMPONENT})
endif()


# osg_import_library(<LIBRARYNAME> <NAME>)
#
# Creates target <LIBRARYNAME> after searching for library named <NAME>.  Creates
# cache entries <LIBRARYNAME>_LIBRARY_DEBUG_NAME and <LIBRARYNAME>_LIBRARY_RELEASE_NAME,
# and sets in parent scope the variable <LIBRARYNAME>_FOUND.  Expects to be able to
# reuse the OSG_LIBRARY_INCLUDE_PATH for includes.
function(import_osg_library LIBRARYNAME NAME)

    find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES ${NAME}d PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
    find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES ${NAME} PATHS ${LIB_DIRS} NO_DEFAULT_PATH)

    # Determine whether we found the library correctly
    if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
        set(${LIBRARYNAME}_FOUND FALSE PARENT_SCOPE)
        mark_as_advanced(CLEAR ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
        return()
    endif()

    mark_as_advanced(FORCE ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    set(${LIBRARYNAME}_FOUND TRUE PARENT_SCOPE)

    # Set the release path, include path, and link libraries
    add_library(${LIBRARYNAME} SHARED IMPORTED)
    set_target_properties(${LIBRARYNAME} PROPERTIES
        IMPORTED_IMPLIB "${${LIBRARYNAME}_LIBRARY_RELEASE_NAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}"
        INTERFACE_COMPILE_OPTIONS "${${LIBRARYNAME}_COMPILE_OPTIONS}"
        INTERFACE_COMPILE_DEFINITIONS "${${LIBRARYNAME}_COMPILE_DEFINITIONS}"
        INTERFACE_LINK_LIBRARIES "${${LIBRARYNAME}_LINK_LIBRARIES}"
    )
    if(${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
        set_target_properties(${LIBRARYNAME} PROPERTIES
            IMPORTED_IMPLIB_DEBUG "${${LIBRARYNAME}_LIBRARY_DEBUG_NAME}"
        )
    endif()
    osg_set_imported_locations_from_implibs(${LIBRARYNAME} ${OSG_DLL_PREFIX})
    if(OSG_SHOULD_INSTALL)
        vsi_install_target(${LIBRARYNAME} ${OSG_INSTALL_COMPONENT})
    endif()

endfunction()

############################################################
# Import each of the OSG helper libraries
import_osg_library(OSGANIMATION osgAnimation)
import_osg_library(OSGDB osgDB)
import_osg_library(OSGFX osgFX)
import_osg_library(OSGGA osgGA)
import_osg_library(OSGMANIPULATOR osgManipulator)
import_osg_library(OSGPARTICLE osgParticle)
import_osg_library(OSGPRESENTATION osgPresentation)
import_osg_library(OSGSHADOW osgShadow)
import_osg_library(OSGSIM osgSim)
import_osg_library(OSGTERRAIN osgTerrain)
import_osg_library(OSGTEXT osgText)
import_osg_library(OSGUTIL osgUtil)
import_osg_library(OSGVIEWER osgViewer)
import_osg_library(OSGVOLUME osgVolume)
import_osg_library(OSGWIDGET osgWidget)

# Only import osgQt if it exists and has a good version
if(QT_FOUND AND EXISTS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgQt/Version")
    file(STRINGS "${${LIBRARYNAME}_LIBRARY_INCLUDE_PATH}/osgQt/Version" OSGQT_DEFINE_VERSION_STR
        REGEX "^# *define[\t ]+OSGQT_QT_VERSION[\t ]+([0-9]+).*")
    string(REGEX REPLACE "^.*OSGQT_QT_VERSION[\t ]+([0-9]+).*$"
        "\\1" OSGQT_QT_VERSION "${OSGQT_DEFINE_VERSION_STR}")
    # Does the version match?
    if(OSGQT_QT_VERSION EQUAL QT_VERSION_MAJOR)
        import_osg_library(OSGQT osgQt)
    else()
        message(STATUS "Omitting osgQt library (v${OSGQT_QT_VERSION}) due to version mismatch with Qt headers (v${QT_VERSION_MAJOR})")
    endif()
endif()

# Plug-ins are found in lib, unless on Windows
if(WIN32)
    set(OS_PLUGIN_SUBDIR "bin")
else()
    if(BUILD_SYSTEM_ARCH STREQUAL "amd64")
        set(OS_PLUGIN_SUBDIR "lib64")
    else()
        set(OS_PLUGIN_SUBDIR "lib")
    endif()
endif()

# Install OSG plugins
set(PLUGIN_DIRS
    $ENV{OSG_DIR}/bin/osgPlugins-${OSG_VERSION}
    $ENV{OSG_DIR}/lib/osgPlugins-${OSG_VERSION}
    $ENV{OSG_DIR}/lib64/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OpenSceneGraph-${OSG_VERSION}/bin/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OpenSceneGraph-${OSG_VERSION}/lib/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OpenSceneGraph-${OSG_VERSION}/lib64/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OpenSceneGraph/${OSG_SUBDIR}/bin/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OpenSceneGraph/${OSG_SUBDIR}/lib/osgPlugins-${OSG_VERSION}
    ${THIRD_DIR}/OpenSceneGraph/${OSG_SUBDIR}/lib64/osgPlugins-${OSG_VERSION}
)

# Find the plugin location
if(WIN32)
    find_path(OSG_PLUGIN_PATH NAMES osgdb_osg.dll PATHS ${PLUGIN_DIRS} NO_DEFAULT_PATH)
else(WIN32)
    find_path(OSG_PLUGIN_PATH NAMES osgdb_osg.so PATHS ${PLUGIN_DIRS} NO_DEFAULT_PATH)
endif(WIN32)

# Put the plugin location in the library list for 32 to 64 but Linux conversion
# so it is properly updated when a 32/64 bit configuration change is made
list(APPEND LIBRARY_LIST "OSG_PLUGIN_PATH")

if(OSG_PLUGIN_PATH)
    mark_as_advanced(FORCE OSG_PLUGIN_PATH)
    set(OPENSCENEGRAPH_FOUND TRUE)
else()
    mark_as_advanced(CLEAR OSG_PLUGIN_PATH)
    set(OPENSCENEGRAPH_FOUND FALSE)
    message(WARNING "OSG Plug-in Path not found.  Will result in improper installation.")
endif()

# Select installation location
if(WIN32)
    set(OSG_PLUGIN_INSTALL_DIR ${INSTALLSETTINGS_RUNTIME_DIR})
else()
    set(OSG_PLUGIN_INSTALL_DIR ${INSTALLSETTINGS_LIBRARY_DIR})
endif()

# Determine which files to install based on Debug or Release Build Type
if(WIN32)
    set(DEBUG_INSTALL_PATTERN "osgdb_.+(_d|d)\\.dll")
    set(RELEASE_INSTALL_PATTERN "osgdb_.+[^_][^d]\\.dll")
else()
    set(DEBUG_INSTALL_PATTERN "osgdb_.+(_d|d)\\.so")
    set(RELEASE_INSTALL_PATTERN "osgdb_.+[^_][^d]\\.so")
endif()

# Set the full install path to the plugin directory
set(OSG_PLUGIN_INSTALL_DIR ${OSG_PLUGIN_INSTALL_DIR}/osgPlugins-${OSG_VERSION})

set(OSG_COMMON_LIBDEPENDENCIES
    OSG OSGDB OSGGA OSGUTIL OSGVIEWER OPENTHREADS
)
set(OSG_ALL_LIBDEPENDENCIES
    OSG OSGDB OSGGA OSGSIM OSGTERRAIN OSGTEXT OSGUTIL OSGVIEWER OSGWIDGET OSGANIMATION OPENTHREADS
)

# Install if INSTALL_THIRDPARTY_LIBRARIES is undefined, or if it is set to true
if(OSG_SHOULD_INSTALL)
    install(DIRECTORY ${OSG_PLUGIN_PATH}/
        DESTINATION ${OSG_PLUGIN_INSTALL_DIR}
        CONFIGURATIONS "Debug"
        COMPONENT ${OSG_INSTALL_COMPONENT}
        FILES_MATCHING REGEX ${DEBUG_INSTALL_PATTERN}
    )

    install(DIRECTORY ${OSG_PLUGIN_PATH}/
        DESTINATION ${OSG_PLUGIN_INSTALL_DIR}
        CONFIGURATIONS "Release"
        COMPONENT ${OSG_INSTALL_COMPONENT}
        FILES_MATCHING REGEX ${RELEASE_INSTALL_PATTERN}
    )
endif()
