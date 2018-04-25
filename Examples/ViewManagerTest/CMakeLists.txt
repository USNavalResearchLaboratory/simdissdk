if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_TEST_VIEWMANAGER)

set(PROJECT_FILES
    TestViewManager.cpp
)

add_executable(example_viewmanagertest ${PROJECT_FILES})
target_link_libraries(example_viewmanagertest PRIVATE simVis simUtil)
set_target_properties(example_viewmanagertest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - View Manager Test"
)
vsi_install_target(example_viewmanagertest SDK_Examples)
