if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(EXAMPLE_QT_DRAG_AND_DROP)

add_executable(example_qtdraganddrop main.cpp)
target_link_libraries(example_qtdraganddrop PRIVATE Qt::Widgets simVis simUtil simQt)
set_target_properties(example_qtdraganddrop PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Drag and Drop"
)
vsi_install_target(example_qtdraganddrop SDK_Examples)
