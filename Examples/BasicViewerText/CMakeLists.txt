if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_BASIC_VIEWER_TEXT)

set(PROJECT_FILES
    BasicViewerText.cpp
)

add_executable(example_basic_viewer_text ${PROJECT_FILES})
target_link_libraries(example_basic_viewer_text PRIVATE simVis simUtil)
set_target_properties(example_basic_viewer_text PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Basic Viewer Text"
)
vsi_install_target(example_basic_viewer_text SDK_Examples)
