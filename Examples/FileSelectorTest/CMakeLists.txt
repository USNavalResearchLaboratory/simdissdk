if(NOT TARGET simQt)
    return()
endif()

project(FileSelectorWidgetTest)

set(GUI_SOURCES
    main.cpp
    TestFileSelectorWidget.h
    TestFileSelectorWidget.cpp
)

VSI_QT_WRAP_UI(Q_FILESELECTOR_FORMS_HEADERS
    TestFileSelectorWidget.ui
)

VSI_QT_WRAP_CPP(Q_FILESELECTOR_HEADERS_MOC
    TestFileSelectorWidget.h
)


VSI_INCLUDE_QT_USE_FILE()

add_executable(example_QtFileSelector ${GUI_SOURCES} ${Q_FILESELECTOR_HEADERS_MOC} ${Q_FILESELECTOR_FORMS_HEADERS})
target_link_libraries(example_QtFileSelector PRIVATE simQt simCore)
target_include_directories(example_QtFileSelector PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtFileSelector PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt File Selector Widget Test"
)
vsi_install_target(example_QtFileSelector SDK_Examples)

VSI_QT_USE_MODULES(example_QtFileSelector LINK_PRIVATE Widgets)
