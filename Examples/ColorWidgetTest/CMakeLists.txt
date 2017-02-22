if(NOT TARGET SIMDIS_SDKQt)
    return()
endif()

project(ColorDialogTest)

set(GUI_SOURCES
    main.cpp
    TestColorWidget.h
    TestColorWidget.cpp
)

VSI_QT_WRAP_UI(Q_COLOR_FORMS_HEADERS
    TestColorWidget.ui
)

VSI_QT_WRAP_CPP(Q_COLOR_HEADERS_MOC
    TestColorWidget.h
)


VSI_INCLUDE_QT_USE_FILE()

add_executable(example_QtColorWidget ${GUI_SOURCES} ${Q_COLOR_HEADERS_MOC} ${Q_COLOR_FORMS_HEADERS})
target_link_libraries(example_QtColorWidget PRIVATE SIMDIS_SDKQt)
target_include_directories(example_QtColorWidget PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtColorWidget PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Color Widget Test"
)
vsi_install_target(example_QtColorWidget SDK_Examples)

VSI_QT_USE_MODULES(example_QtColorWidget LINK_PRIVATE Widgets)
