if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(EXAMPLE_TEST_VIEWMANAGER_QT)

set(PROJECT_FILES
    TestViewManagerQt.cpp
)

add_executable(example_qtviewmanagertest ${PROJECT_FILES})
target_link_libraries(example_qtviewmanagertest PRIVATE simVis simUtil simQt)
set_target_properties(example_qtviewmanagertest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt View Manager Test"
)
vsi_install_target(example_qtviewmanagertest SDK_Examples)

VSI_QT_USE_MODULES(example_qtviewmanagertest LINK_PRIVATE Widgets OpenGL)
