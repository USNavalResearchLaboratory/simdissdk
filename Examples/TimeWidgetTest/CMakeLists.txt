if(NOT TARGET SIMDIS_SDKQt)
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

add_executable(example_QtTimeWidget ${APP_SOURCES} ${Q_TIMEWIDGET_MOC} ${Q_TIMEWIDGET_FORMS})
target_link_libraries(example_QtTimeWidget PRIVATE SIMDIS_SDKQt SIMDIS_SDKCore)
target_include_directories(example_QtTimeWidget PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtTimeWidget PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Time Widget Test"
)
vsi_install_target(example_QtTimeWidget SDK_Examples)

VSI_QT_USE_MODULES(example_QtTimeWidget LINK_PRIVATE Widgets)
