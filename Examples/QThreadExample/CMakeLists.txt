if(NOT TARGET simUtil OR NOT TARGET simQt)
    return()
endif()

project(EXAMPLE_QThread)

set(PROJECT_FILES
    QThreadExample.cpp
    MyMainWindow.h
    MyMainWindow.cpp
    DataGenerator.h
    DataGenerator.cpp
    Reader.h
    Reader.cpp
    Gui.h
    Gui.cpp    
)

VSI_QT_WRAP_CPP(MY_WIDGETS_MOC
    MyMainWindow.h
    Gui.h
    Reader.h
    DataGenerator.h
)

# Convert the .ui file (GUI design from Qt Designer) into a .h file
vsi_qt_wrap_ui(MY_UI QThreadExample.ui)

# Initialize Qt variables
VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_qthreadexample ${PROJECT_FILES} ${MY_WIDGETS_MOC} ${MY_UI})
target_link_libraries(example_qthreadexample PRIVATE simCore simData simVis
    simUtil simQt)
target_include_directories(example_qthreadexample PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qthreadexample PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - QThread Example"
)
vsi_install_target(example_qthreadexample SDK_Examples)

VSI_QT_USE_MODULES(example_qthreadexample LINK_PRIVATE Widgets OpenGL)
