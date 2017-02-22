if(NOT QT_FOUND OR NOT TARGET SIMDIS_SDKQt)
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

add_executable(example_ConsoleDataModel ${PROJECT_FILES} ${MY_WIDGETS_MOC} ${MY_UI})
target_link_libraries(example_ConsoleDataModel PRIVATE SIMDIS_SDKQt SIMDIS_SDKCore)
target_include_directories(example_ConsoleDataModel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_ConsoleDataModel PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Console Data Model"
)
vsi_install_target(example_ConsoleDataModel SDK_Examples)

VSI_QT_USE_MODULES(example_ConsoleDataModel LINK_PRIVATE Widgets)
