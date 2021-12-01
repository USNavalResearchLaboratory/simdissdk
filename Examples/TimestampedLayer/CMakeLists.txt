if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_TIMESTAMPED_LAYER)

set(PROJECT_FILES
    ExampleTimestampedLayer.cpp
)

add_executable(example_timestampedlayer ${PROJECT_FILES})
target_link_libraries(example_timestampedlayer PRIVATE simVis simUtil)
set_target_properties(example_timestampedlayer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Timestamped Layer"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_timestampedlayer PRIVATE ImGuiLib)
    target_compile_definitions(example_timestampedlayer PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_timestampedlayer SDK_Examples)
