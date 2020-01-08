if(NOT TARGET simData OR NOT TARGET simQt OR NOT TARGET simVis)
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

add_executable(example_qtentitylineedittest ${APP_SOURCES} ${Q_EVT_MOC} ${Q_EVT_FORMS})
target_link_libraries(example_qtentitylineedittest PRIVATE simQt simData simVis)
target_include_directories(example_qtentitylineedittest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtentitylineedittest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Entity Line Edit Test"
)
vsi_install_target(example_qtentitylineedittest SDK_Examples)

VSI_QT_USE_MODULES(example_qtentitylineedittest LINK_PRIVATE Widgets)
