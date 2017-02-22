if(NOT TARGET SIMDIS_SDKQt)
    return()
endif()

project(ActionItemModelTest)

set(APP_SOURCES
    ActionItemModelTest.h
    ActionItemModelTest.cpp
)

VSI_QT_WRAP_CPP(Q_AIM_MOC
    ActionItemModelTest.h
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_QtActionItemModel ${APP_SOURCES} ${Q_AIM_MOC})
target_link_libraries(example_QtActionItemModel PRIVATE SIMDIS_SDKQt)
target_include_directories(example_QtActionItemModel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_QtActionItemModel PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Qt Action Item Model"
)
vsi_install_target(example_QtActionItemModel SDK_Examples)

VSI_QT_USE_MODULES(example_QtActionItemModel LINK_PRIVATE Widgets)
