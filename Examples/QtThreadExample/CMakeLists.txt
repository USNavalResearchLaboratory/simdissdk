if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET OSGQT)
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

add_executable(example_qtthreadexample ${PROJECT_FILES} ${MY_WIDGETS_MOC} ${MY_UI})
target_link_libraries(example_qtthreadexample PRIVATE simCore simData simVis
    simUtil simQt)
target_include_directories(example_qtthreadexample PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtthreadexample PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Thread Example"
)
vsi_install_target(example_qtthreadexample SDK_Examples)

VSI_QT_USE_MODULES(example_qtthreadexample LINK_PRIVATE Widgets OpenGL)
