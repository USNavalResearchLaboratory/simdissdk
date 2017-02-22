if(NOT QT_FOUND OR NOT OSGEARTH_QT_FOUND OR NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_QT_DOCKABLE_VIEWS)

set(PROJECT_FILES
    MyMainWindow.h
    QtDockableViews.cpp
)

VSI_QT_WRAP_CPP(MY_WIDGETS_MOC
    MyMainWindow.h
)

# Initialize Qt variables
VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_qtdockableviews ${PROJECT_FILES} ${MY_WIDGETS_MOC})
target_link_libraries(example_qtdockableviews PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil
    OSGEARTH_QT)
target_include_directories(example_qtdockableviews PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtdockableviews PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Dockable Views"
)
vsi_install_target(example_qtdockableviews SDK_Examples)

VSI_QT_USE_MODULES(example_qtdockableviews LINK_PRIVATE Widgets OpenGL)
