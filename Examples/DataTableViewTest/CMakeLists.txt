if(NOT TARGET SIMDIS_SDKQt OR NOT TARGET SIMDIS_SDKData OR NOT TARGET SIMDIS_SDKUtil)
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

add_executable(example_QtDataTableViewTest ${APP_SOURCES} ${Q_DTV_MOC} ${Q_DTV_FORMS})
target_link_libraries(example_QtDataTableViewTest PRIVATE SIMDIS_SDKQt SIMDIS_SDKData SIMDIS_SDKUtil)
target_include_directories(example_QtDataTableViewTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtDataTableViewTest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Data Table View Test"
)
vsi_install_target(example_QtDataTableViewTest SDK_Examples)

VSI_QT_USE_MODULES(example_QtDataTableViewTest LINK_PRIVATE Widgets)
