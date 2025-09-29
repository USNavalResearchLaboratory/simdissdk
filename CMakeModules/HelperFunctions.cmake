# Append values to the list, ensuring that only one instance of each value
# exists within the list.  If the value already exists in the list, it is
# moved to the end of the list.
macro(unique_append TARGET)

    foreach(value ${ARGN})

        # Look for the item in the list
        list(FIND ${TARGET} ${value} index)

        # Remove the item if it was found
        if(NOT index EQUAL -1)
            list(REMOVE_AT ${TARGET} ${index})
        endif()

        # Append the item to the list
        list(APPEND ${TARGET} ${value})

    endforeach()

endmacro()


# Append target string with another string only if the target string
# does not already contain the append string
macro(unique_append_string TARGET APPEND_STRING)
    if(NOT ${TARGET} MATCHES "${APPEND_STRING}[ ]" AND NOT ${TARGET} MATCHES "${APPEND_STRING}$")
        set(${TARGET} "${${TARGET}} ${APPEND_STRING}")
    endif()
endmacro()


# Remove all instances of REMOVE_STRING string from the TARGET string
macro(remove_all_string TARGET REMOVE_STRING)
    STRING(REGEX REPLACE "${REMOVE_STRING}[ ]+" "" ${TARGET} ${${TARGET}})
    STRING(REGEX REPLACE "${REMOVE_STRING}$" "" ${TARGET} ${${TARGET}})
endmacro()


# Disable Console for Release Modes (Release, MinSizeRel, ReleaseWithDebInfo)
macro(disable_win32_console_release EXENAME)
    if(WIN32)
        set(configs ${CMAKE_CONFIGURATION_TYPES})
        list(REMOVE_ITEM configs "Debug")
        foreach(value ${configs})
            string(TOUPPER ${value} config)
            set_target_properties(${EXENAME} PROPERTIES LINK_FLAGS_${config} "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
        endforeach()
    endif()  
endmacro()


# Enable the Console for Debug Mode
macro(enable_win32_console_debug EXENAME)
    if(WIN32)
        set_target_properties(${EXENAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    endif()
endmacro()


# Obtain the target of a symbolic link
# TARGET is an output that will be filled with the target's name if LINK
# is a symbolic link, and will be cleared/unset if LINK is a regular file
function(get_symlink_target LINK TARGET)
    # Start by clearing TARGET
    set(${TARGET} "" PARENT_SCOPE)
    if(WIN32)
        return()
    endif()

    # Obtain file listing for LINK
    execute_process(COMMAND "ls" "-l" "${LINK}" OUTPUT_VARIABLE LISTING)

    # If file listing succeeded, test for regular file or symbolic link
    if(LISTING)
        string(STRIP ${LISTING} LISTING)
        if(${LISTING} MATCHES "^l.*")
            # Obtain the target of the link
            string(REGEX REPLACE ".* -> (.*$)" "\\1" FILENAME ${LISTING})

            # "ls -l" displays target as relative to symbolic link's path
            # Extract the path from the symbolic link and add to the actual name
            string(REGEX MATCH ".*/" FILEPATH ${LINK})
            set(${TARGET} ${FILEPATH}${FILENAME} PARENT_SCOPE)
        endif()
    endif()
endfunction()

# Set <VAR> to the full path of LINK with symbolic links resolved.  <VAR>
# will contain the original LINK along with all symbolic links down to
# the final resolved path.
function(get_symlinks LINK VAR)
    set(TARGETS)
    while(LINK)
        list(APPEND TARGETS ${LINK})
        get_symlink_target(${LINK} LINK)
    endwhile()
    # Set the parent scope value
    set(${VAR} "${TARGETS}" PARENT_SCOPE)
endfunction()

# Wrapper for Qt form files (.ui)
macro(VSI_QT_WRAP_UI DEST)
    QT_WRAP_UI(${DEST} ${ARGN})
    SOURCE_GROUP("Qt\\Form Files" FILES ${ARGN})
    SOURCE_GROUP("Qt\\Generated Files" FILES ${${DEST}})
endmacro()

# Wrapper for header files with the Qt macro QOBJECT
macro(VSI_QT_WRAP_CPP DEST)
    if(NOT CMAKE_AUTOMOC)
        QT_WRAP_CPP(${DEST} ${ARGN})
        SOURCE_GROUP("Qt\\Generated Files" FILES ${${DEST}})
    endif()
endmacro()

# Wrapper for Qt resource files (.qrc)
macro(VSI_QT_ADD_RESOURCES DEST)
    QT_ADD_RESOURCES(${DEST} ${ARGN})
    SOURCE_GROUP("Qt\\Resource Files" FILES ${ARGN})
    SOURCE_GROUP("Qt\\Generated Files" FILES ${${DEST}})
endmacro()

macro(VSI_QT_USE_MODULES TARGET LINK_TYPE)
    set(_CM_LINK_TYPE)
    if("${LINK_TYPE}" STREQUAL "LINK_PUBLIC")
        set(_CM_LINK_TYPE PUBLIC)
    elseif("${LINK_TYPE}" STREQUAL "LINK_PRIVATE")
        set(_CM_LINK_TYPE PRIVATE)
    endif()

    if(Qt5_FOUND OR Qt6_FOUND)
        set(_QT_LIBS ${ARGN})
        # If LINK_TYPE wasn't set to LINK_PUBLIC or LINK_PRIVATE then it's a Qt library
        if(NOT _CM_LINK_TYPE)
            list(APPEND _QT_LIBS ${LINK_TYPE})
        endif()
        # Link against each library specified, e.g. Qt5::Widgets
        foreach(_QT_LIB ${_QT_LIBS})
            target_link_libraries(${TARGET} ${_CM_LINK_TYPE} Qt::${_QT_LIB})
        endforeach()
    else()
        target_link_libraries(${TARGET} ${_CM_LINK_TYPE} ${QT_LIBRARIES})
    endif()
endmacro()

# Errors out if an environment variable is not set. Useful mostly
# for debugging.
# @param envvar Name of the environment variable to check.
macro(initialize_ENV_checked envvar)
    if(NOT DEFINED ENV{${envvar}})
        message(FATAL_ERROR "Environment variable ${envvar} not found.")
    endif()
endmacro()

# If a variable is not set in the environment, then set it to to
# CMake junk, e.g. PEOPLE_DIR=PEOPLE_DIR-NOTFOUND. Otherwise, uses
# of the referenced environment variable will default to the empty
# string (""), which may cause problems if used at the beginning of
# a longer string, e.g. ${PEOPLE_DIR}/lib will resolve to /lib.
# @param  envvar Name of the environment variable to set.
macro(initialize_ENV envvar)
    # Override the environment variable with local variable if set
    if(${envvar})
        set(ENV{${envvar}} ${${envvar}})
    elseif(NOT DEFINED ENV{${envvar}})
        set(ENV{${envvar}} ${envvar}-NOTFOUND)
    endif()
endmacro()

# osg_guess_win32_dll_prefix(<VAR_DLL_PREFIX> <LIB_FILENAME>)
#
# Sets variable <VAR_DLL_PREFIX> to the correct filename prefix for Windows import
# library <LIB_FILENAME>.  It does this by searching the associated "../bin/" directory
# for the DLL associated with LIB_FILENAME, based on a file(GLOB...) search.  For
# example, osg.lib might have an associated DLL osg153-osg.dll.  This routine will
# return the "osg153-" portion of the DLL's filename.  This routine should be reusable
# for OSG and OpenThreads.
function(osg_guess_win32_dll_prefix VAR_DLL_PREFIX LIB_FILENAME)
    set(${VAR_DLL_PREFIX} "" PARENT_SCOPE)
    # Linux returns without any checks
    if(NOT WIN32)
        return()
    endif()
    # Filename should exist
    if(NOT EXISTS "${LIB_FILENAME}")
        message(WARNING "LIB NAME ${LIB_FILENAME} not found...")
        return()
    endif()

    # Find the DLL filename
    get_filename_component(_BIN_DIR ${LIB_FILENAME} DIRECTORY)
    get_filename_component(_NAME_ONLY ${LIB_FILENAME} NAME_WE)
    set(_BIN_DIR "${_BIN_DIR}/../bin/")
    file(GLOB _OUT_FILE RELATIVE "${_BIN_DIR}" "${_BIN_DIR}/*${_NAME_ONLY}.dll")

    # Error check the output -- break out early if file not found
    if(NOT _OUT_FILE)
        message(WARNING "Unable to find DLL file for input LIB ${LIB_FILENAME}")
        return()
    endif()
    # Remove the right "n" characters so that only the prefix remains
    string(LENGTH "${_OUT_FILE}" OUT_FILE_LEN)
    string(LENGTH "${_NAME_ONLY}.dll" CHARS_TO_REMOVE)
    math(EXPR CHARS_TO_KEEP "${OUT_FILE_LEN} - ${CHARS_TO_REMOVE}")
    # Save output in VAR_DLL_PREFIX
    string(SUBSTRING "${_OUT_FILE}" 0 ${CHARS_TO_KEEP} _PREFIX)
    set(${VAR_DLL_PREFIX} "${_PREFIX}" PARENT_SCOPE)
endfunction()

# Sets the IMPORTED_LOCATION and IMPORTED_LOCATION_DEBUG properties for
# TARGET based on TARGET's preexisting IMPORTED_IMPLIB and
# IMPORTED_IMPLIB_DEBUG properties.  On Linux this will typically be a
# string copy, but on Windows the .lib for IMPORTED_IMPLIB will be mangled
# into an acceptable .dll name and location.
function(vsi_set_imported_locations_from_implibs TARGET)
    # Pull out the IMPLIB
    get_target_property(IMPLIB_RELEASE ${TARGET} IMPORTED_IMPLIB)
    if(NOT IMPLIB_RELEASE)
        message(WARNING "${TARGET}: No IMPORTED_IMPLIB, cannot set IMPORTED_LOCATION.  Build may fail.")
        return()
    endif()
    get_target_property(IMPLIB_DEBUG ${TARGET} IMPORTED_IMPLIB_DEBUG)

    # Default the debug location to release location, unless there's a separate debug LOCATION
    set(LOCATION_RELEASE ${IMPLIB_RELEASE})
    set(LOCATION_DEBUG ${IMPLIB_RELEASE})
    if(IMPLIB_DEBUG)
        set(LOCATION_DEBUG ${IMPLIB_DEBUG})
    elseif(WIN32)
        # Warn users on Windows that they might have misconfigured IMPORTED_IMPLIB_DEBUG
        message(AUTHOR_WARNING "${TARGET}: IMPORTED_IMPLIB supplied but no IMPORTED_IMPLIB_DEBUG.  Runtime environment may be invalid for C++ DLLs.")
    endif()

    # On Windows, replace .lib for .dll, then lib/ for bin/
    if(WIN32)
        string(REGEX REPLACE "(.*)\\.lib$" "\\1.dll" LOCATION_RELEASE ${LOCATION_RELEASE})
        string(REGEX REPLACE "(.*)\\.lib$" "\\1.dll" LOCATION_DEBUG ${LOCATION_DEBUG})
        # Convert release, then debug -- .lib to .dll, then lib/lib64 to bin
        string(REGEX REPLACE "/lib(|64)/" "/bin/" LOCATION_RELEASE ${LOCATION_RELEASE})
        string(REGEX REPLACE "/lib(|64)/" "/bin/" LOCATION_DEBUG ${LOCATION_DEBUG})
    endif()

    # Set the imported location now for release and debug
    set_target_properties(${TARGET} PROPERTIES
        IMPORTED_LOCATION "${LOCATION_RELEASE}"
    )
    set_target_properties(${TARGET} PROPERTIES
        IMPORTED_LOCATION_DEBUG "${LOCATION_DEBUG}"
    )
endfunction()

# vsi_set_rpath(<TARGET> <DESTINATION>)
#
# Sets the INSTALL_RPATH property on TARGET based on installing to DESTINATION.
# DESTINATION is typically relative to CMAKE_INSTALL_PREFIX, but may also be
# an absolute path.  The RPATH will be set to pull libs from the
# INSTALLSETTINGS_LIBRARY_DIR value under CMAKE_INSTALL_PREFIX.
function(vsi_set_rpath TARGET DESTINATION)
    if(WIN32)
        return()
    endif()
    if(IS_ABSOLUTE ${DESTINATION})
        set(DESTINATION_ABS ${DESTINATION})
    else()
        set(DESTINATION_ABS ${CMAKE_INSTALL_PREFIX}/${DESTINATION})
    endif()

    # Compute RPATH for lib directory containing dependencies relative to bin installation directory
    file(RELATIVE_PATH REL_INSTALL_PATH ${DESTINATION_ABS} ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
    # If the lib and bin paths were the same, an empty value will be returned
    if(NOT REL_INSTALL_PATH)
        set(REL_INSTALL_PATH ".")
    endif()
    # Set install RPATH variable
    set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH "\$ORIGIN/${REL_INSTALL_PATH}")
endfunction()

# vsi_install_executable(<TARGET> <COMPONENT> [DESTINATION])
#
# Installs executable for compiled TARGET using the installation component
# COMPONENT.  Installs to INSTALLSETTINGS_RUNTIME_DIR unless the DESTINATION
# field is set.
function(vsi_install_executable TARGET COMPONENT)
    set(DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR})
    if(${ARGC} GREATER 2)
        set(DESTINATION ${ARGV2})
    endif()

    # Compute RPATH for UNIX; ignore on Windows
    if(UNIX)
        vsi_set_rpath(${TARGET} ${DESTINATION})
    endif()

    # Setup installation
    install(TARGETS ${TARGET}
        COMPONENT ${COMPONENT}
        RUNTIME DESTINATION ${DESTINATION})
endfunction()

# vsi_install_shared_library(<TARGET> <COMPONENT> [DESTINATION])
#
# Installs compiled shared library TARGET using the installation component
# COMPONENT.  Installs to INSTALLSETTINGS_SHARED_LIBRARY_DIR unless the
# DESTINATION field is set.  Sets RPATH as needed on the target.  If the
# library TARGET has the target property TARGET_EXPORT_NAME set, it will
# install using that value for the install(TARGETS EXPORT) signature.
function(vsi_install_shared_library TARGET COMPONENT)
    set(DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR})
    if(${ARGC} GREATER 2)
        set(DESTINATION ${ARGV2})
    endif()

    # Compute RPATH for UNIX; ignore on Windows
    if(UNIX)
        vsi_set_rpath(${TARGET} ${DESTINATION})
    endif()

    # Pull out custom target property TARGET_EXPORT_NAME
    get_target_property(TARGET_EXPORT_NAME ${TARGET} TARGET_EXPORT_NAME)
    if(NOT TARGET_EXPORT_NAME)
        set(TARGET_EXPORT_NAME ${TARGET}Targets)
    endif()

    install(TARGETS ${TARGET}
        EXPORT ${TARGET_EXPORT_NAME}
        COMPONENT ${COMPONENT}
        RUNTIME DESTINATION ${DESTINATION}  # Windows DLLs
        LIBRARY DESTINATION ${DESTINATION}  # UNIX shared objects
        ARCHIVE DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR}  # Windows LIBs
    )
endfunction()

# vsi_install_plugin(<TARGET> <COMPONENT> [DESTINATION])
#
# Installs compiled plug-in TARGET using the installation component
# COMPONENT.  Installs to INSTALLSETTINGS_PLUGIN_DIR unless the
# DESTINATION field is set.  Sets RPATH as needed on the target.
function(vsi_install_plugin TARGET COMPONENT)
    set(DESTINATION ${INSTALLSETTINGS_PLUGIN_DIR})
    if(${ARGC} GREATER 2)
        set(DESTINATION ${ARGV2})
    endif()
    vsi_install_shared_library(${TARGET} ${COMPONENT} ${DESTINATION})
endfunction()

# vsi_install_imported_shared_library(<TARGET> <COMPONENT> [DESTINATION])
#
# Installs shared library and related symbolic links for TARGET using the
# installation component COMPONENT.  Installs to INSTALLSETTINGS_SHARED_LIBRARY_DIR
# unless the DESTINATION field is set.  Files to install are pulled from the
# IMPORTED_LOCATION property of TARGET.
function(vsi_install_imported_shared_library TARGET COMPONENT)
    set(DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR})
    if(${ARGC} GREATER 2)
        set(DESTINATION ${ARGV2})
    endif()

    # Pull out the imported location
    get_target_property(LOCATION_RELEASE ${TARGET} IMPORTED_LOCATION)
    if(NOT LOCATION_RELEASE)
        get_target_property(LOCATION_RELEASE ${TARGET} IMPORTED_LOCATION_RELEASE)
    endif()
    get_symlinks(${LOCATION_RELEASE} LOCATION_RELEASE)
    install(PROGRAMS ${LOCATION_RELEASE}
        DESTINATION "${DESTINATION}"
        COMPONENT ${COMPONENT}
        CONFIGURATIONS Release)

    # Do the same thing with the debug location
    get_target_property(LOCATION_DEBUG ${TARGET} IMPORTED_LOCATION_DEBUG)
    if(LOCATION_DEBUG)
        get_symlinks(${LOCATION_DEBUG} LOCATION_DEBUG)
        install(PROGRAMS ${LOCATION_DEBUG}
            DESTINATION "${DESTINATION}"
            COMPONENT ${COMPONENT}
            CONFIGURATIONS Debug)
    endif()
endfunction()

# vsi_install_static_library(<TARGET> <COMPONENT> [DESTINATION])
#
# Installs compiled static library for TARGET using the installation component
# COMPONENT.  Installs to INSTALLSETTINGS_LIBRARY_DIR unless the DESTINATION
# parameter is set.  If the library TARGET has the target property
# TARGET_EXPORT_NAME set, it will install using that value for the
# install(TARGETS EXPORT) signature.
function(vsi_install_static_library TARGET COMPONENT)
    set(DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR})
    if(${ARGC} GREATER 2)
        set(DESTINATION ${ARGV2})
    endif()

    # Pull out custom target property TARGET_EXPORT_NAME
    get_target_property(TARGET_EXPORT_NAME ${TARGET} TARGET_EXPORT_NAME)
    if(NOT TARGET_EXPORT_NAME)
        set(TARGET_EXPORT_NAME ${TARGET}Targets)
    endif()

    install(TARGETS ${TARGET}
        EXPORT ${TARGET_EXPORT_NAME}
        COMPONENT ${COMPONENT}
        ARCHIVE DESTINATION ${DESTINATION})
endfunction()

# vsi_install_target(<TARGET> <COMPONENT>)
#
# Installs the TARGET with default settings using the installation component
# COMPONENT.  Determines the appropriate installation location and settings
# based on the TARGET property TYPE and other TARGET properties.
function(vsi_install_target TARGET COMPONENT)
    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    get_target_property(IS_IMPORTED ${TARGET} IMPORTED)
    if(TARGET_TYPE STREQUAL "EXECUTABLE")
        vsi_install_executable(${TARGET} ${COMPONENT})
    elseif(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        if(IS_IMPORTED)
            vsi_install_imported_shared_library(${TARGET} ${COMPONENT})
        else()
            # Don't rely on PREFIX -- it can be changed; instead rely on IS_PLUGIN custom property
            get_target_property(TARGET_IS_PLUGIN ${TARGET} IS_PLUGIN)
            if(TARGET_IS_PLUGIN)
                vsi_install_plugin(${TARGET} ${COMPONENT})
            else()
                vsi_install_shared_library(${TARGET} ${COMPONENT})
            endif()
        endif()
    elseif(TARGET_TYPE STREQUAL "STATIC_LIBRARY")
        vsi_install_static_library(${TARGET} ${COMPONENT})
    else()
        message(WARNING "Unsupported install on target ${TARGET} with type ${TARGET_TYPE}.")
    endif()
endfunction()

# vsi_write_basic_package_config_file(TARGET DEPS)
#
# Given a library TARGET that is being exported, generates a <TARGET>Config.cmake
# file in the build directory for use in vsi_install_export().  This is particularly
# useful in cases where the dependencies of the target are variable and cannot
# be easily hardcoded into a <TARGET>Config.cmake.
function(vsi_write_basic_package_config_file TARGET DEPS)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
"include(CMakeFindDependencyMacro)
set(DEPS \"${DEPS}\")
foreach(DEP IN LISTS DEPS)
    find_dependency(\${DEP})
endforeach()
include(\"\${CMAKE_CURRENT_LIST_DIR}/${TARGET}Targets.cmake\")"
    )
endfunction()

# vsi_install_export(TARGET VERSION COMPATIBILITY)
#
# Given a library TARGET that is being exported, installs a generated <TARGET>Targets.cmake
# file, a generated <TARGET>ConfigVersion.cmake, and either a generated or in-source
# <TARGET>Config.cmake file.  The files get installed to <INSTALLSETTINGS_CMAKE_DIR>/<TARGET>.
# If <TARGET>Config.cmake does not exist, but <TARGET>Config.cmake.in does exist, then
# configure_file() is automatically run on it and that result is copied in instead. This
# convenience means you can use a Config.cmake or Config.cmake.in and either one is
# automatically picked up, with a preference for the Config.cmake file.
#
# The generated Targets file includes the namespace "VSI::".  The library gets installed to
# <INSTALLSETTINGS_SHARED_LIBRARY_DIR> or <INSTALLSETTINGS_LIBRARY_DIR> as appropriate.
#
# This function also creates a new alias target VSI::<TARGET> for ease of use, so that in-source
# builds and out-of-source builds can refer to the target by the same name.
function(vsi_install_export TARGET VERSION COMPATIBILITY)
    install(TARGETS ${TARGET} EXPORT ${TARGET}Targets
        LIBRARY DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
        RUNTIME DESTINATION ${INSTALLSETTINGS_SHARED_LIBRARY_DIR}
        ARCHIVE DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR}
    )
    install(EXPORT ${TARGET}Targets
        FILE ${TARGET}Targets.cmake
        NAMESPACE VSI::
        DESTINATION ${INSTALLSETTINGS_CMAKE_DIR}/${TARGET}
    )

    # Compute RPATH for UNIX; ignore on Windows
    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    if(UNIX AND TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        vsi_set_rpath(${TARGET} ${INSTALLSETTINGS_SHARED_LIBRARY_DIR})
    endif()
    set_target_properties(${TARGET} PROPERTIES VERSION "${VERSION}")

    # Create the ConfigVersion.cmake file for the target
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("${TARGET}ConfigVersion.cmake"
        VERSION ${VERSION}
        COMPATIBILITY ${COMPATIBILITY}
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake"
         DESTINATION ${INSTALLSETTINGS_CMAKE_DIR}/${TARGET}
    )

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}Config.cmake")
        # Use a locally defined <TARGET>Config.cmake if possible
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}Config.cmake"
             DESTINATION ${INSTALLSETTINGS_CMAKE_DIR}/${TARGET})
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}Config.cmake.in")
        # Use configure_file() on the <TARGET>Config.cmake.in file to create the Config.cmake
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}Config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake" @ONLY)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
             DESTINATION ${INSTALLSETTINGS_CMAKE_DIR}/${TARGET})
    else()
        # Create the placeholder file, only once (don't continuously overwrite)
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake")
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake" "include(\"\${CMAKE_CURRENT_LIST_DIR}/${TARGET}Targets.cmake\")\n")
        endif()
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
             DESTINATION ${INSTALLSETTINGS_CMAKE_DIR}/${TARGET})
    endif()

    # Create an alias library
    add_library(VSI::${TARGET} ALIAS ${TARGET})
endfunction()

# vsi_find_package()
#
# Wraps find_package(), returning if VSI::PackageName is already defined.
function(vsi_find_package)
    if(NOT TARGET VSI::${ARGV0})
        find_package(${ARGV})
    endif()
endfunction()

# vsi_require_target(...)
#
# Given a list of targets, calls return() if any target does not exist.
macro(vsi_require_target)
    foreach(target ${ARGN})
        if(NOT TARGET ${target})
            if(WARN_SKIPPED_TARGETS)
                get_filename_component(_DIR "${CMAKE_CURRENT_LIST_DIR}" NAME)
                message(WARNING "Skipping ${_DIR}, missing target ${target}")
            elseif(VERBOSE)
                get_filename_component(_DIR "${CMAKE_CURRENT_LIST_DIR}" NAME)
                message(STATUS "Skipping ${_DIR}, missing target ${target}")
            endif()
            return()
        endif()
    endforeach()
endmacro()

# vsi_add_third_prefix_path(LIBNAME VERSION1 VERSION2 ...)
#
# Given a library LIBNAME and one or more versions, this function will append the
# third party library to the <CMAKE_PREFIX_PATH> only if the directory exists.
# This relies on <THIRD_DIR> being defined, and searches for the library in the
# directory <THIRD_DIR>/<LIBNAME>/<VERSION>. Multiple versions can be specified,
# e.g. to support cases where different architectures support different versions
# of a third party library. If the directory or version is not found, then
# <CMAKE_PREFIX_PATH> is not modified. Only the first found version is applied.
function(vsi_add_third_prefix_path LIBNAME)
    foreach(_LIBVERSION IN ITEMS ${ARGN})
        if(EXISTS "${THIRD_DIR}/${LIBNAME}/${_LIBVERSION}")
            list(APPEND CMAKE_PREFIX_PATH "${THIRD_DIR}/${LIBNAME}/${_LIBVERSION}")
            set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    if(VERBOSE)
        message(WARNING "Did not find appropriate prefix path for ${LIBNAME}, tested versions: ${ARGN}")
    endif()
endfunction()

# vsi_add_qt_prefix_path(VERSION1 ...)
#
# Attempts to find valid installations of Qt based on VSI standards for Qt location, adding
# located versions to the <CMAKE_PREFIX_PATH>. On Windows, Qt is typically found at
# <c:/QtSDK/<BUILD_SYSTEM_CANONICAL_NAME>/VERSION> and on Linux, </usr/local/Qt-VERSION>. The
# user can supply one or more versions of Qt. If found, the directory is appended to
# <CMAKE_PREFIX_PATH> and the function returns. If not found, then a glob() is used to search
# for valid Qt versions, and all are added to the CMAKE_PREFIX_PATH.
function(vsi_add_qt_prefix_path)
    # Define a base directory based on OS where we expect to find Qt libraries
    if(WIN32)
        set(_BASE_QT_PATH "c:/QtSDK/${BUILD_SYSTEM_CANONICAL_NAME}/")
    else()
        set(_BASE_QT_PATH "/usr/local/Qt-")
    endif()
    foreach(_LIBVERSION IN ITEMS ${ARGN})
        if(EXISTS "${_BASE_QT_PATH}${_LIBVERSION}/include/QtCore")
            list(APPEND CMAKE_PREFIX_PATH "${_BASE_QT_PATH}${_LIBVERSION}")
            set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    # Fall back on glob to look for other versions
    file(GLOB _QT_DIRECTORIES "${_BASE_QT_PATH}*.*")
    set(_FOUND_CANDIDATE OFF)
    foreach(_QT_DIRECTORY IN LISTS _QT_DIRECTORIES)
        if(EXISTS "${_QT_DIRECTORY}/include/QtCore")
            list(APPEND CMAKE_PREFIX_PATH "${_QT_DIRECTORY}")
            set(_FOUND_CANDIDATE ON)
        endif()
    endforeach()

    if(_FOUND_CANDIDATE)
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
    elseif(VERBOSE)
        message(WARNING "Did not find expected prefix path for Qt, tested versions: ${ARGN} at ${_BASE_QT_PATH}")
    endif()
endfunction()

# vsi_json_get_value_or_list(<out-var> <json-string> [<member|index> ...])
#
# Get the string or array value in `<json-string>` at the location given by the
# list of `<member|index>` arguments. The `<out-var>` will be set to the
# appropriate value based on data type:
#
#  - String: It is set to the single string value
#  - Array: It is set to a CMake array of strings
#  - Does-not-exist: The value is unset.
function(vsi_json_get_value_or_list VAR JSON)
    string(JSON _VER_TYPE ERROR_VARIABLE _NO_VAL TYPE "${JSON}" ${ARGN})
    if(_NO_VAL)
        unset(${VAR} PARENT_SCOPE)
    elseif(_VER_TYPE STREQUAL "ARRAY")
        # Figure out how many items to iterate
        string(JSON _LEN LENGTH "${JSON}" ${ARGN})
        math(EXPR _LEN "${_LEN}-1")
        # Initialize output value, then loop appending to the output
        set(_LIST)
        foreach(i RANGE ${_LEN})
            string(JSON _VALUE GET "${JSON}" ${ARGN} ${i})
            list(APPEND _LIST ${_VALUE})
        endforeach()
        set(${VAR} ${_LIST} PARENT_SCOPE)
    else()
        string(JSON _VALUE GET "${JSON}" ${ARGN})
        set(${VAR} ${_VALUE} PARENT_SCOPE)
    endif()
endfunction()

# vsi_add_json_prefix_path(<json-string>)
#
# Accepts a JSON object string from the VsiPackages.json format that represents
# a single library, and adds it to the `CMAKE_PREFIX_PATH` using the
# `vsi_add_third_prefix_path()` function. CMake variables defined by the
# Package JSON are also set in the parent scope.
macro(vsi_add_json_prefix_path PACKAGE_JSON_OBJ)
    # Get the package name
    string(JSON _DIR ERROR_VARIABLE _NO_VAL GET "${PACKAGE_JSON_OBJ}" dir)
    if(_NO_VAL)
        string(JSON _DIR ERROR_VARIABLE _NO_VAL GET "${PACKAGE_JSON_OBJ}" name)
    endif()

    # Get the versions list
    vsi_json_get_value_or_list(_VERSIONS "${PACKAGE_JSON_OBJ}" version)
    if(_DIR STREQUAL "QT5")
        vsi_add_qt_prefix_path(${_VERSIONS})
    else()
        vsi_add_third_prefix_path(${_DIR} ${_VERSIONS})
    endif()

    # Look for CMake variables to set
    string(JSON _CMAKE_VARS ERROR_VARIABLE _NO_VAL GET "${PACKAGE_JSON_OBJ}" cmake_vars)
    if(NOT _NO_VAL)
        string(JSON _LEN LENGTH "${_CMAKE_VARS}")
        math(EXPR _LEN "${_LEN}-1")
        foreach(i RANGE ${_LEN})
            string(JSON _VAR_NAME GET "${_CMAKE_VARS}" ${i} name)
            string(JSON _VAR_VALUE GET "${_CMAKE_VARS}" ${i} value)
            set(${_VAR_NAME} "${_VAR_VALUE}")
            if(VERBOSE)
                message(STATUS "Setting ${_VAR_NAME}: ${_VAR_VALUE}")
            endif()
        endforeach()
    endif()
endmacro()

# vsi_process_packages_json(json-filename)
#
# Reads <json-filename> as a VSI Packages JSON file, adding all entries to
# the `CMAKE_PREFIX_PATH` and setting variables defined in the file.
macro(vsi_process_packages_json FILENAME)
    # Read the JSON file.
    file(READ "${FILENAME}" _VSI_PACKAGES_FILE)

    string(JSON _VSI_PACKAGES_JSON GET "${_VSI_PACKAGES_FILE}" packages)
    string(JSON _NUM_PACKAGES LENGTH "${_VSI_PACKAGES_JSON}")
    math(EXPR _NUM_PACKAGES "${_NUM_PACKAGES}-1")

    foreach(i RANGE ${_NUM_PACKAGES})
        string(JSON _PACKAGE_JSON GET "${_VSI_PACKAGES_JSON}" ${i})
        vsi_add_json_prefix_path(${_PACKAGE_JSON})
    endforeach()
    unset(_PACKAGE_JSON)
    unset(_VSI_PACKAGES_JSON)
    unset(_VSI_PACKAGES_FILE)
endmacro()
