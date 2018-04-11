if(NOT TARGET simQt)
    return()
endif()

project(TestWidgetTest)

set(APP_SOURCES
    TimeWidgetTest.cpp
    MainWindow.cpp
)

VSI_QT_WRAP_UI(Q_TIMEWIDGET_FORMS
    MainWindow.ui
)

VSI_QT_WRAP_CPP(Q_TIMEWIDGET_MOC
    MainWindow.h
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_qttimewidgettest ${APP_SOURCES} ${Q_TIMEWIDGET_MOC} ${Q_TIMEWIDGET_FORMS})
target_link_libraries(example_qttimewidgettest PRIVATE simQt simCore)
target_include_directories(example_qttimewidgettest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qttimewidgettest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Time Widget Test"
)
vsi_install_target(example_qttimewidgettest SDK_Examples)

VSI_QT_USE_MODULES(example_qttimewidgettest LINK_PRIVATE Widgets)
