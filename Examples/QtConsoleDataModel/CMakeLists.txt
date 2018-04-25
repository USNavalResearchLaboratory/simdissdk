if(NOT QT_FOUND OR NOT TARGET simQt)
    return()
endif()

project(EXAMPLE_ConsoleDataModel)

set(PROJECT_FILES
    main.cpp
    Console.h
    Console.cpp
)

VSI_INCLUDE_QT_USE_FILE()
VSI_QT_WRAP_CPP(MY_WIDGETS_MOC
    Console.h
)

# Convert the .ui file (GUI design from Qt Designer) into a .h file
vsi_qt_wrap_ui(MY_UI Console.ui)

add_executable(example_qtconsoledatamodel ${PROJECT_FILES} ${MY_WIDGETS_MOC} ${MY_UI})
target_link_libraries(example_qtconsoledatamodel PRIVATE simQt simCore)
target_include_directories(example_qtconsoledatamodel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtconsoledatamodel PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Console Data Model"
)
vsi_install_target(example_qtconsoledatamodel SDK_Examples)

VSI_QT_USE_MODULES(example_qtconsoledatamodel LINK_PRIVATE Widgets)
