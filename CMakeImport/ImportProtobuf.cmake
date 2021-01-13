# Setup PROTOBUF library
# Setting the PROTOBUF_DIR environment variable will allow use of a custom built library

set(PROTOBUF_SUBDIR 3.14.0)

# Setup search paths
initialize_ENV(PROTOBUF_DIR)
set(INCLUDE_DIRS 
    $ENV{PROTOBUF_DIR}/include
    ${THIRD_DIR}/protobuf/${PROTOBUF_SUBDIR}/include
    ${THIRD_DIR}/protobuf-${PROTOBUF_SUBDIR}/include
)

set(LIB_DIRS 
    $ENV{PROTOBUF_DIR}/lib
    ${THIRD_DIR}/protobuf/${PROTOBUF_SUBDIR}/lib
    ${THIRD_DIR}/protobuf-${PROTOBUF_SUBDIR}/lib
)

set(BIN_DIRS 
    $ENV{PROTOBUF_DIR}/bin
    ${THIRD_DIR}/protobuf/${PROTOBUF_SUBDIR}/bin
    ${THIRD_DIR}/protobuf-${PROTOBUF_SUBDIR}/bin
)

find_path(PROTOBUF_LIBRARY_INCLUDE_PATH NAME google/protobuf/descriptor.h PATHS ${INCLUDE_DIRS} NO_DEFAULT_PATH)
find_library(PROTOBUF_LIBRARY_DEBUG_NAME
    NAMES protobuf_d libprotobuf_d protobufd libprotobufd
    HINTS ${LIB_DIRS}
    NO_DEFAULT_PATH
)
find_library(PROTOBUF_LIBRARY_RELEASE_NAME
    NAMES protobuf libprotobuf
    PATHS ${LIB_DIRS}
    NO_DEFAULT_PATH
)

# Determine whether we found the library correctly
if(NOT PROTOBUF_LIBRARY_RELEASE_NAME)
    set(PROTOBUF_FOUND FALSE)
    mark_as_advanced(CLEAR PROTOBUF_LIBRARY_INCLUDE_PATH PROTOBUF_LIBRARY_DEBUG_NAME PROTOBUF_LIBRARY_RELEASE_NAME)
    return()
endif()
# Fall back on release library explicitly, only on Windows
if(WIN32 AND NOT PROTOBUF_LIBRARY_DEBUG_NAME)
    set(PROTOBUF_LIBRARY_DEBUG_NAME "${PROTOBUF_LIBRARY_RELEASE_NAME}" CACHE STRING "Path to a library" FORCE)
endif()

mark_as_advanced(FORCE PROTOBUF_LIBRARY_INCLUDE_PATH PROTOBUF_LIBRARY_DEBUG_NAME PROTOBUF_LIBRARY_RELEASE_NAME)
set(PROTOBUF_FOUND TRUE)

set(PROTOBUF_LIBS)
if(UNIX)
    set(PROTOBUF_LIBS -pthread)
endif()

# Detect whether protobuf is a shared library
set(PROTO_IS_SHARED OFF)
if(WIN32)
    string(REGEX REPLACE "(.*)\\.lib$" "\\1.dll" LOCATION_RELEASE "${PROTOBUF_LIBRARY_RELEASE_NAME}")
    string(REGEX REPLACE "/lib(|64)/" "/bin/" LOCATION_RELEASE "${LOCATION_RELEASE}")
    if(EXISTS "${LOCATION_RELEASE}")
        set(PROTO_IS_SHARED ON)
    endif()
endif()


# Set the release path, include path, and link libraries.  Deal with shared vs static differences
if(PROTO_IS_SHARED)
    add_library(PROTOBUF SHARED IMPORTED)
    set_target_properties(PROTOBUF PROPERTIES
        IMPORTED_IMPLIB "${PROTOBUF_LIBRARY_RELEASE_NAME}"
        INTERFACE_COMPILE_DEFINITIONS "PROTOBUF_USE_DLLS"
    )
    if(PROTOBUF_LIBRARY_DEBUG_NAME)
        set_target_properties(PROTOBUF PROPERTIES
            IMPORTED_IMPLIB_DEBUG "${PROTOBUF_LIBRARY_DEBUG_NAME}"
        )
    endif()
    vsi_set_imported_locations_from_implibs(PROTOBUF)
    if(NOT DEFINED INSTALL_THIRDPARTY_LIBRARIES OR INSTALL_THIRDPARTY_LIBRARIES)
        vsi_install_target(PROTOBUF ThirdPartyLibs)
    endif()
else()
    add_library(PROTOBUF STATIC IMPORTED)
    set_target_properties(PROTOBUF PROPERTIES
        IMPORTED_LOCATION "${PROTOBUF_LIBRARY_RELEASE_NAME}"
    )
    if(PROTOBUF_LIBRARY_DEBUG_NAME)
        set_target_properties(PROTOBUF PROPERTIES
            IMPORTED_LOCATION_DEBUG "${PROTOBUF_LIBRARY_DEBUG_NAME}"
        )
    endif()
endif()
set_target_properties(PROTOBUF PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_LIBRARY_INCLUDE_PATH}"
    INTERFACE_LINK_LIBRARIES "${PROTOBUF_LIBS}"
)


find_program(PROTOBUF_PROTOC NAMES protoc HINTS ${BIN_DIRS})
if(NOT "${PROTOBUF_PROTOC}" MATCHES "-NOTFOUND")
    mark_as_advanced(PROTOBUF_PROTOC)
endif()


# Creates a target responsible for generating protobuf files from output of protoc.  Note that this
# function is used instead of the find_package(Protobuf) function because of limitations in the package's
# output (it can only write to build directory, doesn't seem able to specify DLL exports).
#
# If the PROTOBUF_PROTOC_ARGUMENTS CMake variable exists in this scope, the contents are added as arguments
# to the protoc executable call.  This can be useful for adding extra include directories, for example.
#
# NOTE: This function is also present in SDK/CMakeImport/ImportPROTOBUF.cmake as part of the SIMDIS SDK
#   release.  Please keep it up to date along with this version.
# @param TARGET_NAME Name of a custom target to create that depends on the .proto files
# @param PROTO_DIR Full path to the folder containing .proto files (include path)
# @param PROTO_FILES Path under PROTO_DIR for each protobuf file to compile; relative path
# @param HDR_OUT Output headers to include in your project
# @param SRC_OUT Output source files to include in your project
# @param EXPORT_TYPE Type of cpp_out, e.g. "=dllexport_decl=SDKDATA_EXPORT:." or simply "=."
function(VSI_PROTOBUF_GENERATE TARGET_NAME PROTO_DIR PROTO_FILES HDR_OUT SRC_OUT EXPORT_TYPE)
    if("${PROTOBUF_PROTOC}" MATCHES "-NOTFOUND")
        message(FATAL_ERROR "Unable to find PROTOBUF_PROTOC, which is required for simData.")
    endif()

    if(NOT EXPORT_TYPE)
        set(EXPORT_TYPE =.)
    endif()
    # Keep a list of all the .proto files and all the .pb.h files
    set(_ALL_PROTO)
    set(_ALL_HDR)
    set(_ALL_SRC)

    # Generate each .pb.h and .pb.cc
    foreach(_PROTO ${PROTO_FILES})
        # Strip the filename from the relative path in _PROTO so that we have the relative subdirectory
        string(REGEX MATCH ".*/" _PROTO_SUBDIR ${_PROTO})
        get_filename_component(_PROTO ${PROTO_DIR}/${_PROTO} ABSOLUTE)
        get_filename_component(_PROTO_PATH ${_PROTO} PATH)
        get_filename_component(_PROTO_NAMEONLY ${_PROTO} NAME_WE)
        set(_PROTO_IN_FILE ${_PROTO_PATH}/${_PROTO_NAMEONLY}.proto)
        set(_PROTO_OUT_BASENAME ${CMAKE_CURRENT_BINARY_DIR}/${_PROTO_SUBDIR}${_PROTO_NAMEONLY})

        # Add in command line arguments if they exist
        if(PROTOBUF_PROTOC_ARGUMENTS)
            set(_PROTO_COMMAND_LINE ${PROTOBUF_PROTOC} --cpp_out${EXPORT_TYPE} -I${PROTO_DIR} ${PROTOBUF_PROTOC_ARGUMENTS} ${_PROTO_IN_FILE})
        else()
            set(_PROTO_COMMAND_LINE ${PROTOBUF_PROTOC} --cpp_out${EXPORT_TYPE} -I${PROTO_DIR} ${_PROTO_IN_FILE})
        endif()

        # Define the command that creates the files
        add_custom_command(
            COMMAND ${_PROTO_COMMAND_LINE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${_PROTO_IN_FILE}
            OUTPUT ${_PROTO_OUT_BASENAME}.pb.h ${_PROTO_OUT_BASENAME}.pb.cc
            COMMENT "Generating ${_PROTO_NAMEONLY}.pb.h and ${_PROTO_NAMEONLY}.pb.cc"
        )
        # Save the files for later, for the custom target
        list(APPEND _ALL_PROTO ${_PROTO_IN_FILE})
        list(APPEND _ALL_HDR ${_PROTO_OUT_BASENAME}.pb.h)
        list(APPEND _ALL_SRC ${_PROTO_OUT_BASENAME}.pb.cc)

        # Mark the files as generated
        set_source_files_properties(${_PROTO_OUT_BASENAME}.pb.h ${_PROTO_OUT_BASENAME}.pb.cc PROPERTIES GENERATED TRUE)
    endforeach()

    # Create a target that includes on the .proto files and depends on the header files
    if(NOT "${TARGET_NAME}" STREQUAL "")
        add_custom_target(${TARGET_NAME} ALL
            SOURCES ${_ALL_PROTO}
            DEPENDS ${_ALL_HDR}
        )
    endif()

    # Pass back the hdr and src files
    set(${HDR_OUT} ${_ALL_HDR} PARENT_SCOPE)
    set(${SRC_OUT} ${_ALL_SRC} PARENT_SCOPE)
endfunction()

# Creates .h and .cpp wrappers around the .pb.h and .pb.cc files for the protos passed in.
# @param HDR_OUT Variable name that will hold the list of full paths to all headers that
#    get generated by this functions.
# @param SRC_OUT Variable name that will hold the list of full paths to all source files that
#    get generated by this functions.
# @param ARGN[implied] Uses ARGN for the list of protos to build
function(CREATE_PROTOBUF_WARNINGFREE_WRAPPERS HDR_OUT SRC_OUT)
    set(_ALL_HDR)
    set(_ALL_SRC)
    foreach(_FILE_PREFIX ${ARGN})
        # Figure out where we're writing the new .h and .cpp files
        set(_NEW_H ${CMAKE_CURRENT_BINARY_DIR}/${_FILE_PREFIX}.h)
        set(_NEW_CPP ${CMAKE_CURRENT_BINARY_DIR}/${_FILE_PREFIX}.cpp)
        list(APPEND _ALL_HDR ${_NEW_H})
        list(APPEND _ALL_SRC ${_NEW_CPP})

        # Write the wrappers (which rely on _FILE_PREFIX variable)
        configure_file(${CMAKE_SOURCE_DIR}/CMakeImport/ProtobufFileWrapper.h.in ${_NEW_H} @ONLY)
        set_property(SOURCE ${_NEW_H} PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_FILE_PREFIX}.pb.h)
        configure_file(${CMAKE_SOURCE_DIR}/CMakeImport/ProtobufFileWrapper.cpp.in ${_NEW_CPP} @ONLY)
        set_property(SOURCE ${_NEW_CPP} PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_FILE_PREFIX}.pb.cc)
    endforeach()

    # Pass back the hdr and src files
    set(${HDR_OUT} ${_ALL_HDR} PARENT_SCOPE)
    set(${SRC_OUT} ${_ALL_SRC} PARENT_SCOPE)
endfunction()

# Runs the full circle on protobuf files, running protoc and generating a library target.
# Creates library LIB_TARGETNAME under project LIB_PROJECTNAME.  Uses the FOLDER and
# PROJECT_LABEL to describe the project locations.  Protobuf files are specified relative
# to PROTO_PATH, and are appended to the end of the function (relying on ARGN functionality).
# Protobuf files should be specified without extension.
#
# If the PROTOBUF_PROTOC_ARGUMENTS CMake variable exists in this scope, the contents are added as arguments
# to the protoc executable call.  This can be useful for adding extra include directories, for example.
# See function VSI_PROTOBUF_GENERATE() for details.
function(CREATE_PROTOBUF_LIBRARY LIB_TARGETNAME LIB_PROJECTNAME FOLDER PROJECT_LABEL PROTO_PATH)
    set(_PROTO_PB_CC) # These files won't be added to solution, but are generated.
    set(_PROTO_PB_H)

    # Create a list of proto files with relative path
    set(_PROTO_FILES)
    set(_PROTO_FILES_FULL_PATH)
    foreach(_PROTO_FILE ${ARGN})
        list(APPEND _PROTO_FILES ${_PROTO_FILE}.proto)
        list(APPEND _PROTO_FILES_FULL_PATH ${PROTO_PATH}/${_PROTO_FILE}.proto)
    endforeach()

    # Generate the .pb.cc and .pb.h files from protobuf
    vsi_protobuf_generate(${LIB_TARGETNAME}_Generate
        ${PROTO_PATH}
        "${_PROTO_FILES}"
        _PROTO_PB_H _PROTO_PB_CC
        =.)
    # Set up labeling for the source file project on this target
    set_target_properties(${LIB_TARGETNAME}_Generate PROPERTIES
        FOLDER "${FOLDER}"
        PROJECT_LABEL "${PROJECT_LABEL} Gen")
    # Assign a source group to the generated files
    source_group("Generated Files" FILES ${_PROTO_PB_H})
    source_group("Protobuf Files" FILES ${_PROTO_FILES})

    ##########################################################

    # Generate the header and cpp files that are used to suppress warnings from protobuf
    create_protobuf_warningfree_wrappers(
        _WRAPPERS_H
        _WRAPPERS_CPP
        ${ARGN})

    # Create the Protobuf Code Library
    add_library(${LIB_TARGETNAME} ${_PROTO_FILES_FULL_PATH} ${_PROTO_PB_H} ${_WRAPPERS_H} ${_WRAPPERS_CPP})
    add_dependencies(${LIB_TARGETNAME} ${LIB_TARGETNAME}_Generate)
    target_link_libraries(${LIB_TARGETNAME} PUBLIC PROTOBUF)
    target_include_directories(${LIB_TARGETNAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${INSTALLSETTINGS_INCLUDE_DIR}>
    )
    set_target_properties(${LIB_TARGETNAME} PROPERTIES
        FOLDER "${FOLDER}"
        PROJECT_LABEL "${PROJECT_LABEL}"
        UNITY_BUILD OFF
    )

endfunction()
