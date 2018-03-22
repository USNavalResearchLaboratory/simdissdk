if(NOT TARGET simData OR NOT TARGET simQt)
    return()
endif()

project(EntityLineEditTest)

set(APP_SOURCES
    EntityLineEditTest.cpp
    MainWindow.h
)

VSI_QT_WRAP_CPP(Q_EVT_MOC
    MainWindow.h
)
VSI_QT_WRAP_UI(Q_EVT_FORMS
    MainWindow.ui
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_QtEntityLineEditTest ${APP_SOURCES} ${Q_EVT_MOC} ${Q_EVT_FORMS})
target_link_libraries(example_QtEntityLineEditTest PRIVATE simQt simData)
if(TARGET simVis)
    target_link_libraries(example_QtEntityLineEditTest PRIVATE simVis)
endif()
target_include_directories(example_QtEntityLineEditTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtEntityLineEditTest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Entity Line Edit Test"
)
vsi_install_target(example_QtEntityLineEditTest SDK_Examples)

VSI_QT_USE_MODULES(example_QtEntityLineEditTest LINK_PRIVATE Widgets)
