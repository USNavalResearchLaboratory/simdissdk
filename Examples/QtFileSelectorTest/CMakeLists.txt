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


add_executable(example_qtfileselectortest ${GUI_SOURCES} ${Q_FILESELECTOR_HEADERS_MOC} ${Q_FILESELECTOR_FORMS_HEADERS})
target_link_libraries(example_qtfileselectortest PRIVATE simQt simCore)
target_include_directories(example_qtfileselectortest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtfileselectortest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt File Selector Test"
)
vsi_install_target(example_qtfileselectortest SDK_Examples)

VSI_QT_USE_MODULES(example_qtfileselectortest LINK_PRIVATE Widgets)
