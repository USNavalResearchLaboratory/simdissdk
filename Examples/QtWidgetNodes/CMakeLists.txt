if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(EXAMPLE_QT_WIDGET_NODES)

add_executable(example_qtwidgetnodes main.cpp)
target_link_libraries(example_qtwidgetnodes PRIVATE Qt::Widgets simVis simUtil simQt)
set_target_properties(example_qtwidgetnodes PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Widget Nodes"
)
vsi_install_target(example_qtwidgetnodes SDK_Examples)
