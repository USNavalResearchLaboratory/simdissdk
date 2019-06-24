include(CMakeFindDependencyMacro)
find_dependency(simNotify)
find_dependency(simCore)
include("${CMAKE_CURRENT_LIST_DIR}/simDataTargets.cmake")
