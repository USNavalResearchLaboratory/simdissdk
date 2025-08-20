if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(EXAMPLE_QT_HUD_LABEL_BINS)

add_executable(example_qthudlabelbins main.cpp)
target_link_libraries(example_qthudlabelbins PRIVATE Qt::Widgets simVis simUtil simQt)
set_target_properties(example_qthudlabelbins PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt HUD Label Bins"
)
vsi_install_target(example_qthudlabelbins SDK_Examples)
