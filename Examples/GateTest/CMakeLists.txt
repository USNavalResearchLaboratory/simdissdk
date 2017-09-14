if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_GATE)

set(PROJECT_FILES
    src/GateTest.cpp
)

add_executable(example_gate ${PROJECT_FILES})
target_link_libraries(example_gate PRIVATE simVis simUtil)
set_target_properties(example_gate PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Gates"
)
vsi_install_target(example_gate SDK_Examples)
