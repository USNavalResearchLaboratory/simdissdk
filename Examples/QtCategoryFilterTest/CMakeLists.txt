if(NOT TARGET simData OR NOT TARGET simQt)
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

add_executable(example_qtcategoryfiltertest ${APP_SOURCES} ${Q_EVT_MOC} ${Q_EVT_FORMS})
target_link_libraries(example_qtcategoryfiltertest PRIVATE simQt simData)
target_include_directories(example_qtcategoryfiltertest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtcategoryfiltertest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Category Filter Test"
)
vsi_install_target(example_qtcategoryfiltertest SDK_Examples)

VSI_QT_USE_MODULES(example_qtcategoryfiltertest LINK_PRIVATE Widgets)
