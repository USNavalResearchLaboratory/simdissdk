if(NOT TARGET simUtil OR NOT TARGET simQt)
    return()
endif()

project(EXAMPLE_TEST_VIEWMANAGER_QT)

set(PROJECT_FILES
    TestViewManagerQt.cpp
)

# Initialize Qt variables
VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_viewmanager_qt ${PROJECT_FILES})
target_link_libraries(example_viewmanager_qt PRIVATE simVis simUtil simQt)
set_target_properties(example_viewmanager_qt PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - View Manager Qt"
)
vsi_install_target(example_viewmanager_qt SDK_Examples)

VSI_QT_USE_MODULES(example_viewmanager_qt LINK_PRIVATE Widgets OpenGL)
