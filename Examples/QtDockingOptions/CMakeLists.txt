if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(EXAMPLE_QT_DOCKING_OPTIONS)

set(PROJECT_FILES
    MyMainWindow.h
    QtDockingOptions.cpp
)

VSI_QT_WRAP_CPP(MY_WIDGETS_MOC
    MyMainWindow.h
)

source_group(Qt FILES QtResources.qrc)
VSI_QT_ADD_RESOURCES(Q_RESOURCES QtResources.qrc)

source_group("Generated Files" FILES ${MY_WIDGETS_MOC} ${Q_RESOURCES})

add_executable(example_qtdockingoptions ${PROJECT_FILES} ${MY_WIDGETS_MOC} ${Q_RESOURCES})
target_link_libraries(example_qtdockingoptions PRIVATE simVis simUtil simQt)
target_include_directories(example_qtdockingoptions PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtdockingoptions PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Docking Options"
)
vsi_install_target(example_qtdockingoptions SDK_Examples)
