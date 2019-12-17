if(NOT TARGET simQt OR NOT TARGET simData)
    return()
endif()

project(PLUGIN_SIMQT_DESIGNER_PLUGINS)

# .h files
set(PROJ_HEADERS
    simQtDesignerPlugins.h
    CategoryDataBreadcrumbsPlugin.h
    CategoryFilterWidgetPlugin.h
    ColorButtonPlugin.h
    ColorWidgetPlugin.h
    DataTableComboBoxPlugin.h
    DirectorySelectorWidgetPlugin.h
    DockWidgetPlugin.h
    EntityFilterLineEditPlugin.h
    EntityLineEditPlugin.h
    EntityTreeCompositePlugin.h
    EntityTypeFilterWidgetPlugin.h
    FileSelectorWidgetPlugin.h
    FontWidgetPlugin.h
    GanttChartViewPlugin.h
    SearchLineEditPlugin.h
    TimeButtonsPlugin.h
    TimeWidgetPlugin.h
    UnitsSelectorComboBoxPlugin.h
)

# .cpp files
set(PROJ_SOURCES
    CategoryDataBreadcrumbsPlugin.cpp
    CategoryFilterWidgetPlugin.cpp
    ColorButtonPlugin.cpp
    ColorWidgetPlugin.cpp
    DataTableComboBoxPlugin.cpp
    DirectorySelectorWidgetPlugin.cpp
    DockWidgetPlugin.cpp
    EntityFilterLineEditPlugin.cpp
    EntityLineEditPlugin.cpp
    EntityTreeCompositePlugin.cpp
    EntityTypeFilterWidgetPlugin.cpp
    FileSelectorWidgetPlugin.cpp
    FontWidgetPlugin.cpp
    GanttChartViewPlugin.cpp
    SearchLineEditPlugin.cpp
    TimeButtonsPlugin.cpp
    TimeWidgetPlugin.cpp
    UnitsSelectorComboBoxPlugin.cpp
    simQtDesignerPlugins.cpp
)

if(OSG_FOUND)
    list(APPEND PROJ_HEADERS ColorGradientWidgetPlugin.h)
    list(APPEND PROJ_SOURCES ColorGradientWidgetPlugin.cpp)
endif()

source_group(Headers FILES ${PROJ_HEADERS})
source_group(Sources FILES ${PROJ_SOURCES})

set(PROJ_RESOURCES
    SDKPlugins.qrc
)
source_group(Qt FILES ${PROJ_RESOURCES})

# Must explicitly add the include directories for Designer in Qt5
if(QT5_FOUND)
    if(Qt5UiPlugin_FOUND)
        set(DESIGNER_LIBRARY UiPlugin)
        get_target_property(Qt5UiPlugin_INCLUDE_DIRS Qt5::UiPlugin INTERFACE_INCLUDE_DIRECTORIES)
        include_directories(${Qt5UiPlugin_INCLUDE_DIRS})
    elseif(Qt5Designer_FOUND)
        set(DESIGNER_LIBRARY Designer)
        include_directories(${Qt5Designer_INCLUDE_DIRS})
    else()
        message(STATUS "Qt5 found, but Qt5UiPlugin package not found.  Skipping QtDesigner Plugin.")
        return()
    endif()
endif()

VSI_INCLUDE_QT_USE_FILE(QTDESIGNER)
VSI_QT_ADD_RESOURCES(Q_RESOURCES ${PROJ_RESOURCES})
VSI_QT_WRAP_CPP(Q_HEADERS_MOC ${PROJ_HEADERS})

source_group("Generated Files" FILES ${Q_HEADERS_MOC} ${Q_RESOURCES})

add_library(simQtDesignerWidgets SHARED ${PROJ_SOURCES} ${PROJ_HEADERS}
    ${PROJ_RESOURCES} ${Q_HEADERS_MOC} ${Q_RESOURCES})
target_link_libraries(simQtDesignerWidgets PRIVATE simQt simData)
if(TARGET simVis)
     target_link_libraries(simQtDesignerWidgets PRIVATE simVis)
endif()
target_include_directories(simQtDesignerWidgets PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(simQtDesignerWidgets PRIVATE QDESIGNER_EXPORT_WIDGETS)
set_target_properties(simQtDesignerWidgets PROPERTIES
    FOLDER "Qt Designer Plugins"
    PROJECT_LABEL "Plugin - simQt Designer Widgets")

if(OSG_FOUND)
    target_compile_definitions(simQtDesignerWidgets PRIVATE HAVE_OSG)
endif()

VSI_QT_USE_MODULES(simQtDesignerWidgets LINK_PRIVATE Widgets OpenGL ${DESIGNER_LIBRARY})

option(AUTOINSTALL_QTDESIGNER_WIDGETS "Copies simQt Widgets to Qt Designer plugin directory" OFF)
if(AUTOINSTALL_QTDESIGNER_WIDGETS)
    # After the build, copy the library over
    add_custom_command(TARGET simQtDesignerWidgets POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy $<TARGET_FILE:simQtDesignerWidgets> ${QT_DESIGNER_PLUGIN_DIR}
        )
endif()
