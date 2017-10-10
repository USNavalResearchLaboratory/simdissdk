if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_TIMESTAMPED_LAYER)

set(PROJECT_FILES
    src/ExampleTimestampedLayer.cpp
)

add_executable(example_timestampedlayer ${PROJECT_FILES})
target_link_libraries(example_timestampedlayer PRIVATE simVis simUtil)
set_target_properties(example_timestampedlayer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Timestamped Layer"
)
vsi_install_target(example_timestampedlayer SDK_Examples)
