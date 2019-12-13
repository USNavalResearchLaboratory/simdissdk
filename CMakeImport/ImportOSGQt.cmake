# Setup osgQt library
# Setting the OSGQT_DIR environment variable will allow use of a custom built library

set(OSGQT_FOUND FALSE)

# Cannot locate osgEarth without OSG due to requirement on knowing OSG version
if(NOT OSG_FOUND OR NOT QT_FOUND)
    return()
endif()

set(LIBRARYNAME OSGQT)

# OpenThreads is stored under the OpenSceneGraph folder
set(${LIBRARYNAME}_VERSION ${OSG_VERSION})
set(${LIBRARYNAME}_INSTALL_COMPONENT ThirdPartyLibs)

# Setup search paths
initialize_ENV(OSG_DIR)
initialize_ENV(OSGQT_DIR)
set(INCLUDE_DIRS
    ${OSGQT_DIR}/include
    $ENV{OSGQT_DIR}/include
    ${THIRD_DIR}/osgQt/${${LIBRARYNAME}_VERSION}/include
    ${OSG_DIR}/include
    $ENV{OSG_DIR}/include
    ${THIRD_DIR}/OpenSceneGraph/${${LIBRARYNAME}_VERSION}/include
)

set(LIB_DIRS
    ${OSGQT_DIR}
    $ENV{OSGQT_DIR}
    ${THIRD_DIR}/osgQt/${${LIBRARYNAME}_VERSION}
    ${OSG_DIR}
    $ENV{OSG_DIR}
    ${THIRD_DIR}/OpenSceneGraph/${${LIBRARYNAME}_VERSION}
)

find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH NAME osgQt/Export PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
# Newer versions of osgQt include the Qt version number in output name
find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES osgQt${QT_VERSION_MAJOR}d osgQtd PATHS ${LIB_DIRS} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES osgQt${QT_VERSION_MAJOR} osgQt PATHS ${LIB_DIRS} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)

############################################################
########## Imported 3rd party library boilerplate ##########
############################################################

# Determine whether we found the library correctly
if(NOT ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    set(${LIBRARYNAME}_FOUND FALSE)
    mark_as_advanced(CLEAR ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
    return()
endif()

mark_as_advanced(FORCE ${LIBRARYNAME}_LIBRARY_INCLUDE_PATH ${LIBRARYNAME}_LIBRARY_DEBUG_NAME ${LIBRARYNAME}_LIBRARY_RELEASE_NAME)
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
# Deal with debug
if(${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
    set_target_properties(${LIBRARYNAME} PROPERTIES
        IMPORTED_IMPLIB_DEBUG "${${LIBRARYNAME}_LIBRARY_DEBUG_NAME}"
    )
endif()

# Set the IMPORTED_LOCATION manually.  On Windows, this is a prefix and different directory
if(WIN32)
    # Get the DLL prefix for osgQt.dll, e.g. osg145-
    osg_guess_win32_dll_prefix(${LIBRARYNAME}_DLL_PREFIX "${${LIBRARYNAME}_LIBRARY_RELEASE_NAME}")

    get_filename_component(BASE_PATH ${${LIBRARYNAME}_LIBRARY_RELEASE_NAME} DIRECTORY)
    get_filename_component(NAME_ONLY ${${LIBRARYNAME}_LIBRARY_RELEASE_NAME} NAME_WE)
    set(DLL_PATH "${BASE_PATH}/../bin/${${LIBRARYNAME}_DLL_PREFIX}${NAME_ONLY}.dll")
    get_filename_component(DLL_PATH "${DLL_PATH}" ABSOLUTE)
    set_target_properties(${LIBRARYNAME} PROPERTIES IMPORTED_LOCATION "${DLL_PATH}")

    # Same thing, but for the debug name
    if(${LIBRARYNAME}_LIBRARY_DEBUG_NAME)
        get_filename_component(BASE_PATH ${${LIBRARYNAME}_LIBRARY_DEBUG_NAME} DIRECTORY)
        get_filename_component(NAME_ONLY ${${LIBRARYNAME}_LIBRARY_DEBUG_NAME} NAME_WE)
        set(DLL_PATH "${BASE_PATH}/../bin/${${LIBRARYNAME}_DLL_PREFIX}${NAME_ONLY}.dll")
        get_filename_component(DLL_PATH "${DLL_PATH}" ABSOLUTE)
        set_target_properties(${LIBRARYNAME} PROPERTIES IMPORTED_LOCATION_DEBUG "${DLL_PATH}")
    endif()
else()
    vsi_set_imported_locations_from_implibs(${LIBRARYNAME})
endif()


# Install if INSTALL_THIRDPARTY_LIBRARIES is undefined, or if it is set to true
if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
    vsi_install_target(${LIBRARYNAME} ${${LIBRARYNAME}_INSTALL_COMPONENT})
endif()
