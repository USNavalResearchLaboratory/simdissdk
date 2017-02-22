if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_GATE)

set(PROJECT_FILES
    src/GateTest.cpp
)

add_executable(example_gate ${PROJECT_FILES})
target_link_libraries(example_gate PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_gate PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Gates"
)
vsi_install_target(example_gate SDK_Examples)
