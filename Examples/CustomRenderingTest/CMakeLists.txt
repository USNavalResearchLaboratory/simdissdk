project(EXAMPLE_CUSTOM_RENDERING_TEST)

set(PROJECT_FILES
    CustomRenderingTest.cpp
)

add_executable(example_customrenderingtest ${PROJECT_FILES})
target_link_libraries(example_customrenderingtest PRIVATE simVis simUtil)
set_target_properties(example_customrenderingtest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Custom Rendering"
)
vsi_install_target(example_customrenderingtest SDK_Examples)
