# ----- simQt -----------------------------------------------------

if(NOT QT_FOUND)
    message(FATAL "Cannot configure simQt without Qt.")
endif()

set(SIMQT_INC)
set(SIMQT_SRC)

# .h files
set(SIMQT_HEADERS_TO_MOC
    ${SIMQT_INC}AbstractEntityTreeModel.h
    ${SIMQT_INC}ActionRegistry.h
    ${SIMQT_INC}ActionItemModel.h
    ${SIMQT_INC}BoundSettings.h
    ${SIMQT_INC}ColorButton.h
    ${SIMQT_INC}ColorWidget.h
    ${SIMQT_INC}ConsoleDataModel.h
    ${SIMQT_INC}ConsoleLogger.h
    ${SIMQT_INC}DirectorySelectorWidget.h
    ${SIMQT_INC}DockWidget.h
    ${SIMQT_INC}FileDescriptorReplacement.h
    ${SIMQT_INC}FileSelectorWidget.h
    ${SIMQT_INC}FontWidget.h
    ${SIMQT_INC}GanttChartView.h
    ${SIMQT_INC}MruList.h
    ${SIMQT_INC}PersistentFileLogger.h
    ${SIMQT_INC}SearchLineEdit.h
    ${SIMQT_INC}SegmentedSpinBox.h
    ${SIMQT_INC}SegmentedTexts.h
    ${SIMQT_INC}SettingsItemDelegate.h
    ${SIMQT_INC}SettingsModel.h
    ${SIMQT_INC}SettingsProxyModel.h
    ${SIMQT_INC}SliderTimeWidgetBinding.h
    ${SIMQT_INC}StartupLayoutTask.h
    ${SIMQT_INC}StdStreamConsoleChannel.h
    ${SIMQT_INC}TimeButtons.h
    ${SIMQT_INC}TimeFormatContainer.h
    ${SIMQT_INC}TimeSliderClockBinding.h
    ${SIMQT_INC}TimeWidget.h
    ${SIMQT_INC}UnitContext.h
)
set(SIMQT_HEADERS
    ${SIMQT_INC}ConsoleChannel.h
    ${SIMQT_INC}FileDialog.h
    ${SIMQT_INC}FileUtilities.h
    ${SIMQT_INC}MonospaceItemDelegate.h
    ${SIMQT_INC}QtFormatting.h
    ${SIMQT_INC}ResourceInitializer.h
    ${SIMQT_INC}ScopedSignalBlocker.h
    ${SIMQT_INC}Settings.h
    ${SIMQT_INC}SettingsGroup.h
    ${SIMQT_INC}StartupLayoutManager.h
    ${SIMQT_INC}UnitsComboBox.h
    ${SIMQT_INC}WeightedMenuManager.h
    ${SIMQT_INC}WidgetSettings.h
)

# Parts of simQt depend on simData
if(TARGET simData)
    list(APPEND SIMQT_HEADERS_TO_MOC
        ${SIMQT_INC}CategoryFilterWidget.h
        ${SIMQT_INC}CategoryTreeModel.h
        ${SIMQT_INC}DataTableComboBox.h
        ${SIMQT_INC}DataTableModel.h
        ${SIMQT_INC}EntityCategoryFilter.h
        ${SIMQT_INC}EntityFilter.h
        ${SIMQT_INC}EntityFilterLineEdit.h
        ${SIMQT_INC}EntityLineEdit.h
        ${SIMQT_INC}EntityProxyModel.h
        ${SIMQT_INC}EntityStateFilter.h
        ${SIMQT_INC}EntityTreeComposite.h
        ${SIMQT_INC}EntityTreeWidget.h
        ${SIMQT_INC}EntityTypeFilter.h
        ${SIMQT_INC}EntityTypeFilterWidget.h
        ${SIMQT_INC}TimeWidgetClockBinding.h
    )
endif()

# Parts of simQt depend on simVis
if(TARGET simVis)
    list(APPEND SIMQT_HEADERS_TO_MOC
        ${SIMQT_INC}CenterEntity.h
        ${SIMQT_INC}EntityNameFilter.h
        ${SIMQT_INC}EntityTreeModel.h
        ${SIMQT_INC}MapDataModel.h
        ${SIMQT_INC}TimestampedLayerManager.h
        ${SIMQT_INC}ViewManagerDataModel.h
    )
    list(APPEND SIMQT_HEADERS
        ${SIMQT_INC}QtConversion.h
    )
endif()

list(APPEND SIMQT_HEADERS
    ${SIMQT_HEADERS_TO_MOC}
)


source_group(Headers FILES ${SIMQT_HEADERS})
set(SIMQT_ALL_HEADER
    ${SIMQT_INC}../simQt.h
)
source_group("" FILES ${SIMQT_ALL_HEADER})

# .cpp files
set(SIMQT_SOURCES
    ${SIMQT_SRC}ActionRegistry.cpp
    ${SIMQT_SRC}ActionItemModel.cpp
    ${SIMQT_SRC}BoundSettings.cpp
    ${SIMQT_SRC}ColorButton.cpp
    ${SIMQT_SRC}ColorWidget.cpp
    ${SIMQT_SRC}ConsoleChannel.cpp
    ${SIMQT_SRC}ConsoleDataModel.cpp
    ${SIMQT_SRC}ConsoleLogger.cpp
    ${SIMQT_SRC}DirectorySelectorWidget.cpp
    ${SIMQT_SRC}DockWidget.cpp
    ${SIMQT_SRC}FileDescriptorReplacement.cpp
    ${SIMQT_SRC}FileDialog.cpp
    ${SIMQT_SRC}FileSelectorWidget.cpp
    ${SIMQT_SRC}FileUtilities.cpp
    ${SIMQT_SRC}FontWidget.cpp
    ${SIMQT_SRC}GanttChartView.cpp
    ${SIMQT_SRC}MonospaceItemDelegate.cpp
    ${SIMQT_SRC}MruList.cpp
    ${SIMQT_SRC}PersistentFileLogger.cpp
    ${SIMQT_SRC}ResourceInitializer.cpp
    ${SIMQT_SRC}SearchLineEdit.cpp
    ${SIMQT_SRC}SegmentedSpinBox.cpp
    ${SIMQT_SRC}SegmentedTexts.cpp
    ${SIMQT_SRC}Settings.cpp
    ${SIMQT_SRC}SettingsGroup.cpp
    ${SIMQT_SRC}SettingsItemDelegate.cpp
    ${SIMQT_SRC}SettingsModel.cpp
    ${SIMQT_SRC}SettingsProxyModel.cpp
    ${SIMQT_SRC}SliderTimeWidgetBinding.cpp
    ${SIMQT_SRC}StartupLayoutManager.cpp
    ${SIMQT_SRC}StartupLayoutTask.cpp
    ${SIMQT_SRC}StdStreamConsoleChannel.cpp
    ${SIMQT_SRC}TimeButtons.cpp
    ${SIMQT_SRC}TimeFormatContainer.cpp
    ${SIMQT_SRC}TimeSliderClockBinding.cpp
    ${SIMQT_SRC}TimeWidget.cpp
    ${SIMQT_SRC}UnitsComboBox.cpp
    ${SIMQT_SRC}UnitContext.cpp
    ${SIMQT_SRC}WeightedMenuManager.cpp
    ${SIMQT_SRC}WidgetSettings.cpp
)

# Parts of simQt depend on simData
if(TARGET simData)
    list(APPEND SIMQT_SOURCES
        ${SIMQT_SRC}CategoryFilterWidget.cpp
        ${SIMQT_SRC}CategoryTreeModel.cpp
        ${SIMQT_SRC}DataTableComboBox.cpp
        ${SIMQT_SRC}DataTableModel.cpp
        ${SIMQT_SRC}EntityCategoryFilter.cpp
        ${SIMQT_SRC}EntityFilterLineEdit.cpp
        ${SIMQT_SRC}EntityLineEdit.cpp
        ${SIMQT_SRC}EntityProxyModel.cpp
        ${SIMQT_SRC}EntityStateFilter.cpp
        ${SIMQT_SRC}EntityTreeComposite.cpp
        ${SIMQT_SRC}EntityTreeWidget.cpp
        ${SIMQT_SRC}EntityTypeFilter.cpp
        ${SIMQT_SRC}EntityTypeFilterWidget.cpp
        ${SIMQT_SRC}TimeWidgetClockBinding.cpp
    )
endif()

# Parts of simQt depend on simVis
if(TARGET simVis)
    list(APPEND SIMQT_SOURCES
        ${SIMQT_SRC}CenterEntity.cpp
        ${SIMQT_SRC}EntityNameFilter.cpp
        ${SIMQT_SRC}EntityTreeModel.cpp
        ${SIMQT_SRC}MapDataModel.cpp
        ${SIMQT_SRC}QtConversion.cpp
        ${SIMQT_SRC}TimestampedLayerManager.cpp
        ${SIMQT_SRC}ViewManagerDataModel.cpp
    )
endif()

source_group(Sources FILES ${SIMQT_SOURCES})

# .ui files
set(SIMQT_FORMS
    ${SIMQT_INC}ColorWidget.ui
    ${SIMQT_INC}DataTableComboBox.ui
    ${SIMQT_INC}DirectorySelectorWidget.ui
    ${SIMQT_INC}EntityLineEdit.ui
    ${SIMQT_INC}EntityTreeComposite.ui
    ${SIMQT_INC}EntityTypeFilter.ui
    ${SIMQT_INC}FileSelectorWidget.ui
    ${SIMQT_INC}FontWidget.ui
    ${SIMQT_INC}SearchLineEdit.ui
    ${SIMQT_INC}TimeButtons.ui
)
set(SIMQT_RESOURCES
    ${SIMQT_INC}simQtResources.qrc
)
source_group(Qt FILES ${SIMQT_FORMS} ${SIMQT_RESOURCES})

# Wrap files into generated versions
VSI_INCLUDE_QT_USE_FILE()
VSI_QT_WRAP_CPP(Q_HEADERS ${SIMQT_HEADERS_TO_MOC})
VSI_QT_WRAP_UI(Q_FORMS ${SIMQT_FORMS})
VSI_QT_ADD_RESOURCES(Q_RESOURCES ${SIMQT_RESOURCES})

source_group("Generated Files" FILES ${Q_HEADERS} ${Q_FORMS} ${Q_RESOURCES})

set(SIMQT_PROJECT_FILES
    ${SIMQT_HEADERS} ${SIMQT_ALL_HEADER} ${SIMQT_SOURCES} ${SIMQT_FORMS} ${SIMQT_RESOURCES}
    ${Q_HEADERS} ${Q_FORMS} ${Q_RESOURCES}
)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMQT_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(simQt ${STATIC_OR_SHARED} ${SIMQT_PROJECT_FILES})
set_target_properties(simQt PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "SIMDIS SDK - Qt"
    OUTPUT_NAME SIMDIS_SDKQt
)
target_include_directories(simQt
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
if(TARGET simData)
    target_include_directories(simQt PUBLIC
        $<BUILD_INTERFACE:${PROTOBUF_LIBRARY_INCLUDE_PATH}>
    )
endif()
if(TARGET simVis)
    target_include_directories(simQt PUBLIC
        $<BUILD_INTERFACE:${OSG_LIBRARY_INCLUDE_PATH}>
        $<BUILD_INTERFACE:${OSGEARTH_LIBRARY_INCLUDE_PATH}>
    )
endif()

target_link_libraries(simQt PUBLIC simCore simNotify)
# Parts of simQt depend on simData's protobuf generated code
if(TARGET simData)
    add_dependencies(simQt simData_protobuf_generation)
endif()

if(UNIX)
    # UNIX libs that link against simQt need to specify X11 lib on command line
    target_link_libraries(simQt PUBLIC X11)
endif()

if(SIMQT_SHARED)
    target_compile_definitions(simQt PRIVATE simQt_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(simQt PUBLIC simQt_LIB_EXPORT_STATIC)
endif()
if(TARGET simData)
    target_compile_definitions(simQt PRIVATE HAVE_SIMDATA)
endif()

# If simQt is a DLL, avoid linking in "heavy" dependencies if possible
if(SIMQT_SHARED OR NOT SIMVIS_SHARED OR NOT SIMDATA_SHARED)
    if(TARGET simData)
        target_link_libraries(simQt PUBLIC simData)
    endif()
    if(TARGET simVis)
        target_link_libraries(simQt PRIVATE simVis)
    endif()
else()
    if(TARGET simData)
        target_include_directories(simQt PUBLIC $<BUILD_INTERFACE:${SIMDIS_SDK_BINARY_DIR}/simData>)
    endif()
    if(TARGET simVis)
        target_include_directories(simQt PRIVATE ${SIMDIS_SDK_BINARY_DIR}/simVis/include)
    endif()
endif()


VSI_QT_USE_MODULES(simQt LINK_PUBLIC Widgets)

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    set_target_properties(simQt PROPERTIES TARGET_EXPORT_NAME "SIMDIS_SDK")
    vsi_install_target(simQt SDK_Library)
endif()

if(AUTOINSTALL_LIBS AND SIMQT_SHARED)
    PostBuildInstallSdkSharedObjects(simQt)
endif()

if(INSTALL_HEADERS)
    # Setup header file installation
    install(FILES ${SIMQT_HEADERS}
        DESTINATION include/simQt
    )

    # simQt.h
    install(FILES ${SIMQT_ALL_HEADER}
        DESTINATION include
    )
endif()
