#[=======================================================================[.rst:
AddProtoLibrary
---------------

Utility functions to easily create static or shared libraries out of protobuf files.
This is intended to replace the older CreateProtobufLibrary code.s

.. command:: add_proto_library

  Adds a new library (static or shared) that contains the generated protobuf C++ code,
  along with warning-free headers:

  .. code-block:: cmake

    add_proto_library(
      <target-name>
      [STATIC|SHARED]
      [LITE]
      [APPEND_PATH]
      [PROTO_DIR <dir>]
      [EXPORT_MACRO <macro>]
      [PROTOC_OUT_DIR <dir>]
      [HDR_CODE_BLOCK <cpp-code>]
      [PROTOC_OPTIONS <option>...]
      [IMPORT_DIRS <dir>...]
      [<proto-file>...])


 `SHARED` libraries can specify an `EXPORT_MACRO` 

  ``<target-name>``
    Name of the target to create as a library.

  ``STATIC``
    Create the library as a static library. This is the default.

  ``SHARED``
    Create the library as a shared library. Use in conjunction with ``EXPORT_MACRO``
    for best success on Windows.

  ``LITE``
    Link against the libprotobuf-lite library. Only valid for ``STATIC``

  ``APPEND_PATH``
    Pass-throguh to ``protobuf-generate``: A flag that causes the base path of all proto
    schema files to be added to ``IMPORT_DIRS``. This may be useful when the proto files
    are in subdirectories relative to the loaded CMakeLists.txt file.

  ``NO_HEADER``
    Do not generate the warning-free headers. By default, all headers are generated.

  ``PROTO_DIR <dir>``
    Directory relative to ``CMAKE_CURRENT_SOURCE_DIR`` where protobuf files ``proto-file``
    are located. Often this is omitted, but sometimes is set to ``proto`` or ``GenCode``.
    This is automatically added to ``IMPORT_DIRS``.

  ``EXPORT_MACRO <macro>``
    Is a macro which should expand to ``__declspec(dllexport)`` or
    ``__declspec(dllimport)`` depending on what is being compiled.
    Only relevant for ``SHARED`` library invocations.

  ``PROTOC_OUT_DIR <dir>``
    Directory for writing protoc output. Defaults to ``CMAKE_CURRENT_BINARY_DIR``.

  ``HDR_CODE_BLOCK <cpp-code>``
    Add the provided C++ code block to the top of warning-free headers generated.

  ``PROTOC_OPTIONS <option>...``
    Additional arguments that are forwarded to protoc.

  ``IMPORT_DIRS <dir>...``
    Additional import directories for protoc to use when processing the import keyword.
    The ``PROTO_DIR`` if specified is automatically added to this.

  ``<proto-file>...``
    ``.proto`` files

#]=======================================================================]

function(add_proto_library TARGETNAME)
    # Must have the protobuf_generate() command from find_package(Protobuf)
    if(NOT COMMAND protobuf_generate)
        find_package(Protobuf REQUIRED)
    endif()

    set(_options SHARED STATIC LITE NO_HEADER APPEND_PATH)
    set(_singleargs PROTO_DIR EXPORT_MACRO PROTO_OUT_DIR HDR_CODE_BLOCK)
    set(_multiargs PROTOC_OPTIONS IMPORT_DIRS)
    cmake_parse_arguments(arg "${_options}" "${_singleargs}" "${_multiargs}" ${ARGN})

    # Must have the protobuf_generate() command from find_package(Protobuf)
    if(NOT COMMAND protobuf_generate)
        find_package(Protobuf REQUIRED)
    endif()

    # Arguments that will be passed to protobuf_generate(), built throughout the function
    set(_GEN_ARGS
        TARGET ${TARGETNAME}
        LANGUAGE cpp
    )

    # APPEND_PATH is a pass-through
    if(arg_APPEND_PATH)
        list(APPEND _GEN_ARGS APPEND_PATH)
    endif()

    # Determine if static or shared; default to static
    set(_LIB_TYPE STATIC)
    if(arg_SHARED)
        set(_LIB_TYPE SHARED)
        # Shared libraries might need export macros
        if(arg_EXPORT_MACRO)
            list(APPEND _GEN_ARGS EXPORT_MACRO ${arg_EXPORT_MACRO})
        endif()
    endif()

    # Argument for PROTO_OUT_DIR to protobuf_generate
    set(_PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    if(arg_PROTO_OUT_DIR)
        set(_PROTO_OUT_DIR ${arg_PROTO_OUT_DIR})
        list(APPEND _GEN_ARGS PROTO_OUT_DIR ${arg_PROTO_OUT_DIR})
    endif()

    # Add the optional arguments
    if(arg_PROTOC_OPTIONS)
        list(APPEND _GEN_ARGS PROTOC_OPTIONS ${arg_PROTOC_OPTIONS})
    endif()

    # Make sure protobuf directory has a trailing slash
    set(_PROTO_DIR)
    if(arg_PROTO_DIR)
        set(_PROTO_DIR "${arg_PROTO_DIR}/")
        list(APPEND arg_IMPORT_DIRS "${_PROTO_DIR}")
    endif()

    # IMPORT_DIRS passes directly down and is used for import processing
    if(arg_IMPORT_DIRS)
        list(APPEND _GEN_ARGS IMPORT_DIRS ${arg_IMPORT_DIRS})
    endif()

    # Put together the warning-free headers and list of files to include in the library
    set(_PROTO_EXT)
    set(_HDR_FILES)
    set(PROTOBUF_WRAPPER_PREFACE ${arg_HDR_CODE_BLOCK})
    foreach(_FILE_PREFIX IN LISTS arg_UNPARSED_ARGUMENTS)
        # Generate a warning-free wrapper header in the output directory
        if(NOT arg_NO_HEADER)
            set(_HDR_OUT ${_PROTO_OUT_DIR}/${_FILE_PREFIX}.h)
            configure_file(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ProtobufFileWrapper.h.in ${_HDR_OUT} @ONLY)
            set_property(SOURCE ${_HDR_OUT} PROPERTY OBJECT_DEPENDS ${_PROTO_OUT_DIR}/${PROTO_FILE_PREFIX}.pb.h)
            list(APPEND _HDR_FILES ${_HDR_OUT})
        endif()

        list(APPEND _PROTO_EXT ${_PROTO_DIR}${_FILE_PREFIX}.proto)
    endforeach()

    # Create the library itself
    add_library(${TARGETNAME} ${_LIB_TYPE} ${_PROTO_EXT} ${_HDR_FILES})
    if(arg_LITE AND NOT arg_SHARED)
        target_link_libraries(${TARGETNAME} PUBLIC protobuf::libprotobuf-lite)
    else()
        target_link_libraries(${TARGETNAME} PUBLIC protobuf::libprotobuf)
    endif()
    target_include_directories(${TARGETNAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${INSTALLSETTINGS_INCLUDE_DIR}>
    )
    # Cannot build .pb.cc files in unity mode
    set_target_properties(${TARGETNAME} PROPERTIES UNITY_BUILD OFF)
    # Run the generation on the .proto files, generating .pb.h/.pb.cc files
    protobuf_generate(${_GEN_ARGS})

    # Do the compile definitions for declspec
    if(arg_SHARED AND arg_EXPORT_MACRO)
        if(WIN32)
            target_compile_definitions(${TARGETNAME}
                PRIVATE "${arg_EXPORT_MACRO}=__declspec(dllexport)"
                INTERFACE "${arg_EXPORT_MACRO}=__declspec(dllimport)"
            )
        else()
            target_compile_definitions(${TARGETNAME}
                PUBLIC "${arg_EXPORT_MACRO}="
            )
        endif()
    endif()

    # Enable /bigobj on Windows in debug mode to avoid segments issue
    if(MSVC AND arg_SHARED)
        target_compile_options(${TARGETNAME} PRIVATE "$<$<CONFIG:Debug,RelWithDebInfo>:-bigobj>")
    endif()
endfunction()
