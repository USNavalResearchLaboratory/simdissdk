if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(EXAMPLE_QT_DOCKABLE_VIEWS)

set(PROJECT_FILES
    MyMainWindow.h
    QtDockableViews.cpp
)

VSI_QT_WRAP_CPP(MY_WIDGETS_MOC
    MyMainWindow.h
)

add_executable(example_qtdockableviews ${PROJECT_FILES} ${MY_WIDGETS_MOC})
target_link_libraries(example_qtdockableviews PRIVATE simVis simUtil simQt)
target_include_directories(example_qtdockableviews PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtdockableviews PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Dockable Views"
)
vsi_install_target(example_qtdockableviews SDK_Examples)

VSI_QT_USE_MODULES(example_qtdockableviews LINK_PRIVATE Widgets OpenGL)
