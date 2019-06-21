# Finds and imports Protocol Buffers library.

# First try to check PROTOBUF_DIR from PublicDefaults.cmake / CMake Cache
if(PROTOBUF_DIR)
    # Configure the protobuf variables for find_package(Protobuf) to work
    find_library(PROTOBUF_LIBRARY
        NAMES protobuf-2.6 libprotobuf-2.6 protobuf libprotobuf
        PATHS "${PROTOBUF_DIR}/lib" NO_DEFAULT_PATH)
    find_library(PROTOBUF_LIBRARY_DEBUG
        NAMES protobuf-2.6_d libprotobuf-2.6_d protobuf_d libprotobuf_d
        PATHS "${PROTOBUF_DIR}/lib" NO_DEFAULT_PATH)
    find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc PATHS "${PROTOBUF_DIR}/bin" NO_DEFAULT_PATH)
    find_path(PROTOBUF_INCLUDE_DIR NAME google/protobuf/stubs/common.h PATHS "${PROTOBUF_DIR}/include" NO_DEFAULT_PATH)

    # Determine whether we found the library correctly
    if(NOT PROTOBUF_LIBRARY)
        set(PROTOBUF_FOUND FALSE)
        mark_as_advanced(CLEAR PROTOBUF_INCLUDE_DIR PROTOBUF_LIBRARY PROTOBUF_LIBRARY_DEBUG PROTOBUF_PROTOC_EXECUTABLE)
    else()
        mark_as_advanced(FORCE PROTOBUF_INCLUDE_DIR PROTOBUF_LIBRARY PROTOBUF_LIBRARY_DEBUG PROTOBUF_PROTOC_EXECUTABLE)
        set(PROTOBUF_FOUND TRUE)
    endif()
endif()

# Defer to the find_package() to fill out any variables that we don't have from defaults
if(NOT PROTOBUF_FOUND)
    # Note that this may change PROTOBUF_FOUND value
    find_package(Protobuf)
endif()

# Tell end user if there is a problem
if(NOT PROTOBUF_FOUND)
    message(WARNING "Did not find protobuf, required for building simData and its dependencies.")
    return()
endif()

# We only care about the targets for libprotobuf -- create the target
add_library(PROTOBUF SHARED IMPORTED)
set_target_properties(PROTOBUF PROPERTIES
    IMPORTED_LOCATION "${PROTOBUF_LIBRARY}"
    IMPORTED_IMPLIB "${PROTOBUF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PROTOBUF_INCLUDE_DIR}"
)
if(PROTOBUF_LIBRARY_DEBUG)
    set_target_properties(PROTOBUF PROPERTIES
        IMPORTED_LOCATION_DEBUG "${PROTOBUF_LIBRARY_DEBUG}"
        IMPORTED_IMPLIB_DEBUG "${PROTOBUF_LIBRARY_DEBUG}"
    )
endif()
# On UNIX, protobuf depends on pthreads
if(UNIX)
    set_target_properties(PROTOBUF PROPERTIES INTERFACE_LINK_LIBRARIES "-pthread")
endif()

# Detect if the library is a DLL by looking for its DLL
if(WIN32)
    string(REGEX REPLACE "(.*)\\.lib$" "\\1.dll" LOCATION_RELEASE "${PROTOBUF_LIBRARY}")
    string(REGEX REPLACE "/lib(|64)/" "/bin/" LOCATION_RELEASE "${LOCATION_RELEASE}")
    if(EXISTS "${LOCATION_RELEASE}")
        set_target_properties(PROTOBUF PROPERTIES INTERFACE_COMPILE_DEFINITIONS "PROTOBUF_USE_DLLS")
        # Need to set correct imported locations
        vsi_set_imported_locations_from_implibs(PROTOBUF)
        if(INSTALL_THIRDPARTY_LIBRARIES)
            vsi_install_target(PROTOBUF THIRDPARTY)
        endif()
    endif()
endif()


# Save the executable name as a separate variable used in VSI_PROTOBUF_GENERATE_TARGET_NAME
set(PROTOBUF_PROTOC ${PROTOBUF_PROTOC_EXECUTABLE})



# Creates a target responsible for generating protobuf files from output of protoc.  Note that this
# function is used instead of the find_package(Protobuf) function because of limitations in the package's
# output (it can only write to build directory, doesn't seem able to specify DLL exports).
#
# If the PROTOBUF_PROTOC_ARGUMENTS CMake variable exists in this scope, the contents are added as arguments
# to the protoc executable call.  This can be useful for adding extra include directories, for example.
# @param TARGET_NAME Name of a custom target to create that depends on the .proto files
# @param PROTO_DIR Full path to the folder containing .proto files (include path)
# @param PROTO_FILES Path under PROTO_DIR for each protobuf file to compile; relative path
# @param HDR_OUT Output headers to include in your project
# @param SRC_OUT Output source files to include in your project
# @param EXPORT_TYPE Type of cpp_out, e.g. "=dllexport_decl=SDKDATA_EXPORT:." or simply "=."
function(VSI_PROTOBUF_GENERATE TARGET_NAME PROTO_DIR PROTO_FILES HDR_OUT SRC_OUT EXPORT_TYPE)
    if("${PROTOBUF_PROTOC}" MATCHES "-NOTFOUND")
        message(FATAL_ERROR "Unable to find PROTOBUF_PROTOC, which is required for simData.")
    endif("${PROTOBUF_PROTOC}" MATCHES "-NOTFOUND")

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
