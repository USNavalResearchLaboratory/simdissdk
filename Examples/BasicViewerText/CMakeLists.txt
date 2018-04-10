if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_BASIC_VIEWER_TEXT)

set(PROJECT_FILES
    BasicViewerText.cpp
)

add_executable(example_basicviewertext ${PROJECT_FILES})
target_link_libraries(example_basicviewertext PRIVATE simVis simUtil)
set_target_properties(example_basicviewertext PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Basic Viewer Text"
)
vsi_install_target(example_basicviewertext SDK_Examples)
