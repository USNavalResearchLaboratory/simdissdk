# Setup OpenThreads library
# Setting the OSG_DIR or OPENTHREADS_DIR environment variable will allow use of a custom built library

set(LIBRARYNAME OPENTHREADS)

# OpenThreads is stored under the OpenSceneGraph folder
set(${LIBRARYNAME}_VERSION 3.4.0)
# Adjust for the Qt version.  Note that 4.8 version works across the board for 4.8.x
set(${LIBRARYNAME}_VERSION ${${LIBRARYNAME}_VERSION}_Qt-${QT_VERSION})
# Prefix on the DLL under Windows
set(${LIBRARYNAME}_DLL_PREFIX ot20-)
set(${LIBRARYNAME}_INSTALL_COMPONENT ThirdPartyLibs)

# Compile options: OPENTHREADS depends on PThreads on UNIX
if(UNIX)
    set(${LIBRARYNAME}_COMPILE_OPTIONS -pthread)
    set(${LIBRARYNAME}_LINK_LIBRARIES -pthread)
endif()

# Setup search paths
initialize_ENV(OSG_DIR)
initialize_ENV(OPENTHREADS_DIR)
set(INCLUDE_DIRS 
    $ENV{OPENTHREADS_DIR}/include
    ${OSG_DIR}
    $ENV{OSG_DIR}/include
    ${THIRD_DIR}/OpenSceneGraph/${${LIBRARYNAME}_VERSION}/include
)

set(LIB_DIRS 
    $ENV{OPENTHREADS_DIR}/lib
    $ENV{OPENTHREADS_DIR}/lib64
    ${OSG_DIR}/lib
    ${OSG_DIR}/lib64
    $ENV{OSG_DIR}/lib
    $ENV{OSG_DIR}/lib64
    ${THIRD_DIR}/OpenSceneGraph/${${LIBRARYNAME}_VERSION}/lib
    ${THIRD_DIR}/OpenSceneGraph/${${LIBRARYNAME}_VERSION}/lib64
)


find_path(${LIBRARYNAME}_LIBRARY_INCLUDE_PATH NAME OpenThreads/Version PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_DEBUG_NAME NAMES OpenThreadsd PATHS ${LIB_DIRS} NO_DEFAULT_PATH)
find_library(${LIBRARYNAME}_LIBRARY_RELEASE_NAME NAMES OpenThreads PATHS ${LIB_DIRS} NO_DEFAULT_PATH)

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
