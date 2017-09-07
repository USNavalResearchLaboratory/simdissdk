if(NOT QT_FOUND OR NOT OSGEARTH_QT_FOUND OR NOT TARGET simUtil OR NOT TARGET simQt)
    return()
endif()

project( EXAMPLE_TEST_VIEWMANAGER_DATAMODEL_QT )

VSI_QT_WRAP_CPP( PROJECT_MOC_FILES MainWindow.h )
set(PROJECT_FILES
    TestViewManagerQt.cpp
    MainWindow.h
)

# Initialize Qt variables
VSI_INCLUDE_QT_USE_FILE(QTOPENGL)

add_executable(example_viewmanager_datamodel_qt ${PROJECT_FILES} ${PROJECT_MOC_FILES})
target_link_libraries(example_viewmanager_datamodel_qt PRIVATE simVis simUtil
    simQt OSGEARTH_QT)
set_target_properties(example_viewmanager_datamodel_qt PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - View Manager Qt Data Model"
)
vsi_install_target(example_viewmanager_datamodel_qt SDK_Examples)

VSI_QT_USE_MODULES(example_viewmanager_datamodel_qt LINK_PRIVATE Widgets OpenGL)
