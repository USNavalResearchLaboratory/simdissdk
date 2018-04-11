if(NOT TARGET simQt OR NOT TARGET simData OR NOT TARGET simUtil)
    return()
endif()

project(DataTableViewTest)

set(APP_SOURCES
    DataTableViewTest.cpp
    MainWindow.h
)

VSI_QT_WRAP_CPP(Q_DTV_MOC
    MainWindow.h
)
VSI_QT_WRAP_UI(Q_DTV_FORMS
    MainWindow.ui
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_qtdatatableviewtest ${APP_SOURCES} ${Q_DTV_MOC} ${Q_DTV_FORMS})
target_link_libraries(example_qtdatatableviewtest PRIVATE simQt simData simUtil)
target_include_directories(example_qtdatatableviewtest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtdatatableviewtest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Data Table View Test"
)
vsi_install_target(example_qtdatatableviewtest SDK_Examples)

VSI_QT_USE_MODULES(example_qtdatatableviewtest LINK_PRIVATE Widgets)
