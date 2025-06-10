if(NOT Protobuf_FOUND)
    # CONFIG mode needs 22+ to work well with protobuf_generate
    find_package(Protobuf 5.22 CONFIG QUIET)
    if(NOT Protobuf_FOUND)
        include(CMakeFindDependencyMacro)
        find_dependency(Protobuf)
    endif()
endif()
include("${CMAKE_CURRENT_LIST_DIR}/simDataProtoTargets.cmake")
