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

add_executable(example_qtentityviewtest ${APP_SOURCES} ${Q_EVT_MOC} ${Q_EVT_FORMS})
target_link_libraries(example_qtentityviewtest PRIVATE simQt simData)
if(TARGET simVis)
    target_link_libraries(example_qtentityviewtest PRIVATE simVis)
endif()
target_include_directories(example_qtentityviewtest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtentityviewtest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Entity View Test"
)
vsi_install_target(example_qtentityviewtest SDK_Examples)

VSI_QT_USE_MODULES(example_qtentityviewtest LINK_PRIVATE Widgets)
