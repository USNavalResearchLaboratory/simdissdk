include(CMakeFindDependencyMacro)
find_dependency(Protobuf)
include("${CMAKE_CURRENT_LIST_DIR}/simDataProtoTargets.cmake")
