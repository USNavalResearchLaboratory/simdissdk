if(NOT TARGET simUtil OR NOT TARGET simQt)
    return()
endif()

project(EXAMPLE_TEST_VIEWMANAGER_QT)

set(PROJECT_FILES
    TestViewManagerQt.cpp
)

# Initialize Qt variables
VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_viewmanagerqttest ${PROJECT_FILES})
target_link_libraries(example_viewmanagerqttest PRIVATE simVis simUtil simQt)
set_target_properties(example_viewmanagerqttest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - View Manager Qt Test"
)
vsi_install_target(example_viewmanagerqttest SDK_Examples)

VSI_QT_USE_MODULES(example_viewmanagerqttest LINK_PRIVATE Widgets OpenGL)
