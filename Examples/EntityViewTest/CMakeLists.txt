if(NOT TARGET simData OR NOT TARGET simQt)
    return()
endif()

project(EntityViewTest)

set(APP_SOURCES
    EntityViewTest.cpp
    MainWindow.h
)

VSI_QT_WRAP_CPP(Q_EVT_MOC
    MainWindow.h
)
VSI_QT_WRAP_UI(Q_EVT_FORMS
    MainWindow.ui
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_QtEntityViewTest ${APP_SOURCES} ${Q_EVT_MOC} ${Q_EVT_FORMS})
target_link_libraries(example_QtEntityViewTest PRIVATE simQt simData)
if(TARGET simVis)
    target_link_libraries(example_QtEntityViewTest PRIVATE simVis)
endif()
target_include_directories(example_QtEntityViewTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtEntityViewTest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Entity View Test"
)
vsi_install_target(example_QtEntityViewTest SDK_Examples)

VSI_QT_USE_MODULES(example_QtEntityViewTest LINK_PRIVATE Widgets)
