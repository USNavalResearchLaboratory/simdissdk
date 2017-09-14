if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_RFPROP)

set(PROJECT_FILES
    RFProp.cpp
)

add_executable(example_rfprop ${PROJECT_FILES})
target_link_libraries(example_rfprop PRIVATE simVis simUtil)
set_target_properties(example_rfprop PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - RF Propagation"
)
vsi_install_target(example_rfprop SDK_Examples)
