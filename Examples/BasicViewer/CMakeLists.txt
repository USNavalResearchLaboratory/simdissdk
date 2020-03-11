if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_BASIC_VIEWER)

set(PROJECT_FILES
    BasicViewer.cpp
)

add_executable(example_basicviewer ${PROJECT_FILES})
target_link_libraries(example_basicviewer PRIVATE simVis simUtil)
set_target_properties(example_basicviewer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Basic Viewer"
)
vsi_install_target(example_basicviewer SDK_Examples)
