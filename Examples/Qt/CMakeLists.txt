if(NOT QT_FOUND OR NOT OSGEARTH_QT_FOUND OR NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_QT)

set(PROJECT_FILES
    src/ExampleQt.cpp
)

VSI_QT_WRAP_CPP(MY_WIDGETS_MOC
    src/MyMainWindow.h
)

# Initialize Qt variables
VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_qt ${PROJECT_FILES} ${MY_WIDGETS_MOC})
target_link_libraries(example_qt PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil OSGEARTH_QT)
target_include_directories(example_qt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qt PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Integration"
)
vsi_install_target(example_qt SDK_Examples)

VSI_QT_USE_MODULES(example_qt LINK_PRIVATE Widgets OpenGL)
