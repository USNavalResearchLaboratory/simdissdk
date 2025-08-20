if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

# This example has not yet been upgraded to Qt6 due to QTouchDevice
if(Qt6Core_FOUND)
    return()
endif()

project(EXAMPLE_QT_TOUCH_VIEWER)

VSI_QT_WRAP_CPP(PROJECT_MOC_FILES MainWindow.h)
set(PROJECT_FILES
    QtTouchViewer.cpp
    MainWindow.h
)

add_executable(example_qttouchviewer ${PROJECT_FILES} ${PROJECT_MOC_FILES})
target_link_libraries(example_qttouchviewer PRIVATE simVis simUtil simQt)
set_target_properties(example_qttouchviewer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Touch Viewer"
)
vsi_install_target(example_qttouchviewer SDK_Examples)

VSI_QT_USE_MODULES(example_qttouchviewer LINK_PRIVATE Widgets OpenGL)
