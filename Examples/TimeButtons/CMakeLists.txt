if(NOT TARGET simQt OR NOT TARGET simUtil)
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

VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_QtTimeButtons ${APP_SOURCES} ${Q_TIMEBUTTONS_MOC} ${Q_TIMEBUTTONS_FORMS})
target_link_libraries(example_QtTimeButtons PRIVATE simQt simCore
    simVis simUtil)
target_include_directories(example_QtTimeButtons PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtTimeButtons PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Time Buttons Test"
)
vsi_install_target(example_QtTimeButtons SDK_Examples)

VSI_QT_USE_MODULES(example_QtTimeButtons LINK_PRIVATE Widgets OpenGL)

