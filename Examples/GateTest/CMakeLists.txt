if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_GATE)

set(PROJECT_FILES
    GateTest.cpp
)

add_executable(example_gatetest ${PROJECT_FILES})
target_link_libraries(example_gatetest PRIVATE simVis simUtil)
set_target_properties(example_gatetest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Gates Test"
)
vsi_install_target(example_gatetest SDK_Examples)
