if(NOT TARGET simUtil OR NOT TARGET simQt OR NOT TARGET osgQt::osgQOpenGL)
    return()
endif()

project( EXAMPLE_TEST_VIEWMANAGER_DATAMODEL_QT )

VSI_QT_WRAP_CPP( PROJECT_MOC_FILES MainWindow.h )
set(PROJECT_FILES
    TestViewManagerQt.cpp
    MainWindow.h
)

add_executable(example_qtviewmanagerdatamodel ${PROJECT_FILES} ${PROJECT_MOC_FILES})
target_link_libraries(example_qtviewmanagerdatamodel PRIVATE simVis simUtil simQt)
set_target_properties(example_qtviewmanagerdatamodel PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt View Manager Data Model"
)
vsi_install_target(example_qtviewmanagerdatamodel SDK_Examples)

VSI_QT_USE_MODULES(example_qtviewmanagerdatamodel LINK_PRIVATE Widgets OpenGL)
