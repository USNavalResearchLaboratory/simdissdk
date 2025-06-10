if(NOT TARGET simQt OR NOT TARGET simUtil OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project(ExampleTimeButtons)

set(APP_SOURCES
    TimeButtons.cpp
    MainWindow.cpp
)

VSI_QT_WRAP_CPP(Q_TIMEBUTTONS_MOC
    MainWindow.h
)

add_executable(example_qttimebuttons ${APP_SOURCES} ${Q_TIMEBUTTONS_MOC} ${Q_TIMEBUTTONS_FORMS})
target_link_libraries(example_qttimebuttons PRIVATE simQt simCore
    simVis simUtil)
target_include_directories(example_qttimebuttons PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qttimebuttons PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Time Buttons"
)
vsi_install_target(example_qttimebuttons SDK_Examples)

VSI_QT_USE_MODULES(example_qttimebuttons LINK_PRIVATE Widgets OpenGL)
