if(NOT TARGET simQt)
    return()
endif()

project(UnitsWidgetTest)

set(GUI_SOURCES
    UnitsComboBoxTest.h
    UnitsComboBoxTest.cpp
)

VSI_QT_WRAP_UI(Q_UNITS_FORMS_HEADERS
    UnitsComboBoxTest.ui
)

VSI_QT_WRAP_CPP(Q_UNITS_HEADERS_MOC
    UnitsComboBoxTest.h
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(example_qtunitswidgettest ${GUI_SOURCES} ${Q_UNITS_FORMS_HEADERS} ${Q_UNITS_HEADERS_MOC})
target_link_libraries(example_qtunitswidgettest PRIVATE simQt)
target_include_directories(example_qtunitswidgettest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(example_qtunitswidgettest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Qt Units Widget Test"
)
vsi_install_target(example_qtunitswidgettest SDK_Examples)

VSI_QT_USE_MODULES(example_qtunitswidgettest LINK_PRIVATE Widgets)
