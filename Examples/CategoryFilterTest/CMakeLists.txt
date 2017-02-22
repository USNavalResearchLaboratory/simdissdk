if(NOT TARGET SIMDIS_SDKData OR NOT TARGET SIMDIS_SDKQt)
    return()
endif()

project(CategoryFilterTest)

set(APP_SOURCES
    CategoryFilterTest.cpp
    MainWindow.h
)

VSI_QT_WRAP_CPP(Q_EVT_MOC
    MainWindow.h
)
VSI_QT_WRAP_UI(Q_EVT_FORMS
    MainWindow.ui
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_QtCategoryFilterTest ${APP_SOURCES} ${Q_EVT_MOC} ${Q_EVT_FORMS})
target_link_libraries(example_QtCategoryFilterTest PRIVATE SIMDIS_SDKQt SIMDIS_SDKData)
target_include_directories(example_QtCategoryFilterTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtCategoryFilterTest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Category Filter Test"
)
vsi_install_target(example_QtCategoryFilterTest SDK_Examples)

VSI_QT_USE_MODULES(example_QtCategoryFilterTest LINK_PRIVATE Widgets)
