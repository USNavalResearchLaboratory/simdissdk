if(NOT TARGET simQt)
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

add_executable(example_qtactionitemmodeltest ${APP_SOURCES} ${Q_AIM_MOC})
target_link_libraries(example_qtactionitemmodeltest PRIVATE simQt)
target_include_directories(example_qtactionitemmodeltest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtactionitemmodeltest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Action Item Model Test"
)
vsi_install_target(example_qtactionitemmodeltest SDK_Examples)

VSI_QT_USE_MODULES(example_qtactionitemmodeltest LINK_PRIVATE Widgets)
