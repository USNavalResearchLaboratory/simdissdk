# ----- simQt -----------------------------------------------------

if(NOT Qt5Core_FOUND)
    message(FATAL_ERROR "Cannot configure simQt without Qt.")
endif()

set(SIMQT_INC)
set(SIMQT_SRC)

# Variables used in the simQtConfig.cmake generation
set(SIMQT_HAVE_SIMDATA OFF)
set(SIMQT_HAVE_OSGQT OFF)
set(SIMQT_HAVE_SIMVIS OFF)
set(SIMQT_HAVE_SIMUTIL OFF)

# .h files
set(SIMQT_HEADERS_TO_MOC
    ${SIMQT_INC}AbstractEntityTreeModel.h
    ${SIMQT_INC}ActionRegistry.h
    ${SIMQT_INC}ActionItemModel.h
    ${SIMQT_INC}BoundSettings.h
    ${SIMQT_INC}ColorButton.h
    ${SIMQT_INC}ColorWidget.h
    ${SIMQT_INC}ColorWidgetDelegate.h
    ${SIMQT_INC}ConsoleDataModel.h
    ${SIMQT_INC}ConsoleLogger.h
    ${SIMQT_INC}DirectorySelectorWidget.h
    ${SIMQT_INC}DndTreeView.h
    ${SIMQT_INC}DockWidget.h
    ${SIMQT_INC}FileDescriptorReplacement.h
    ${SIMQT_INC}FileSelectorWidget.h
    ${SIMQT_INC}FontWidget.h
    ${SIMQT_INC}GanttChartView.h
    ${SIMQT_INC}KeySequenceEdit.h
    ${SIMQT_INC}MruList.h
    ${SIMQT_INC}PersistentFileLogger.h
    ${SIMQT_INC}SearchLineEdit.h
    ${SIMQT_INC}SegmentedSpinBox.h
    ${SIMQT_INC}SegmentedTexts.h
    ${SIMQT_INC}SettingsItemDelegate.h
    ${SIMQT_INC}SettingsModel.h
    ${SIMQT_INC}SettingsProxyModel.h
    ${SIMQT_INC}SliderTimeWidgetBinding.h
    ${SIMQT_INC}SplashScreen.h
    ${SIMQT_INC}StartupLayoutTask.h
    ${SIMQT_INC}StdStreamConsoleChannel.h
    ${SIMQT_INC}TimeButtons.h
    ${SIMQT_INC}TimeFormatContainer.h
    ${SIMQT_INC}TimeSliderClockBinding.h
    ${SIMQT_INC}TimeWidget.h
    ${SIMQT_INC}Toast.h
    ${SIMQT_INC}UnitContext.h
    ${SIMQT_INC}UnitsComboBox.h
)
set(SIMQT_HEADERS
    ${SIMQT_INC}ConsoleChannel.h
    ${SIMQT_INC}FileDialog.h
    ${SIMQT_INC}FileUtilities.h
    ${SIMQT_INC}MonospaceItemDelegate.h
    ${SIMQT_INC}QtFormatting.h
    ${SIMQT_INC}QtUtils.h
    ${SIMQT_INC}ResourceInitializer.h
    ${SIMQT_INC}ScopedSignalBlocker.h
    ${SIMQT_INC}Settings.h
    ${SIMQT_INC}SettingsGroup.h
    ${SIMQT_INC}StartupLayoutManager.h
    ${SIMQT_INC}ValueMerge.h
    ${SIMQT_INC}WeightedMenuManager.h
    ${SIMQT_INC}WidgetSettings.h
)

# Parts of simQt depend on simData
if(TARGET simData)
    set(SIMQT_HAVE_SIMDATA ON)
    list(APPEND SIMQT_HEADERS_TO_MOC
        ${SIMQT_INC}CategoryDataBreadcrumbs.h
        ${SIMQT_INC}CategoryFilterCounter.h
        ${SIMQT_INC}CategoryFilterWidget.h
        ${SIMQT_INC}CategoryTreeModel.h
        ${SIMQT_INC}DataTableComboBox.h
        ${SIMQT_INC}EntityCategoryFilter.h
        ${SIMQT_INC}EntityFilter.h
        ${SIMQT_INC}EntityFilterLineEdit.h
        ${SIMQT_INC}EntityNameFilter.h
        ${SIMQT_INC}EntityProxyModel.h
        ${SIMQT_INC}EntityStateFilter.h
        ${SIMQT_INC}EntityTreeComposite.h
        ${SIMQT_INC}EntityTreeModel.h
        ${SIMQT_INC}EntityTreeWidget.h
        ${SIMQT_INC}EntityTypeFilter.h
        ${SIMQT_INC}EntityTypeFilterWidget.h
        ${SIMQT_INC}TimeWidgetClockBinding.h
    )

    list(APPEND SIMQT_HEADERS
        ${SIMQT_INC}RegExpImpl.h
    )
endif()

# Parts of simQt depend on simVis
if(TARGET simVis)
    set(SIMQT_HAVE_SIMVIS ON)
    list(APPEND SIMQT_HEADERS_TO_MOC
        ${SIMQT_INC}CenterEntity.h
        ${SIMQT_INC}EntityLineEdit.h
        ${SIMQT_INC}MapDataModel.h
        ${SIMQT_INC}MultiTouchEventFilter.h
        ${SIMQT_INC}TimestampedLayerManager.h
        ${SIMQT_INC}ViewManagerDataModel.h
    )
    list(APPEND SIMQT_HEADERS
        ${SIMQT_INC}FillItemModelWithNodeVisitor.h
    )

    # Parts of simQt depend on both simVis and osgQt
    if (TARGET OSGQT)
        set(SIMQT_HAVE_OSGQT ON)
        list(APPEND SIMQT_HEADERS_TO_MOC
            ${SIMQT_INC}ViewWidget.h
        )
        list(APPEND SIMQT_HEADERS
            ${SIMQT_INC}Gl3FormatGuesser.h
        )
    endif()
endif()

if(OSG_FOUND)
    list(APPEND SIMQT_HEADERS_TO_MOC
        ${SIMQT_INC}ArticulationsEditorWidget.h
        ${SIMQT_INC}ColorGradientWidget.h
    )
    list(APPEND SIMQT_HEADERS
        ${SIMQT_INC}ColorGradient.h
        ${SIMQT_INC}MergeValues.h
        ${SIMQT_INC}QtConversion.h
    )
endif()

if(TARGET simUtil)
    set(SIMQT_HAVE_SIMUTIL ON)
endif()

# Parts of simQt depend on simUtil and simData
if(TARGET simUtil AND TARGET simData)
    list(APPEND SIMQT_HEADERS_TO_MOC
        ${SIMQT_INC}DataTableModel.h
    )
endif()

list(APPEND SIMQT_HEADERS
    ${SIMQT_HEADERS_TO_MOC}
)


source_group(Headers FILES ${SIMQT_HEADERS})

# .cpp files
set(SIMQT_SOURCES
    ${SIMQT_SRC}ActionRegistry.cpp
    ${SIMQT_SRC}ActionItemModel.cpp
    ${SIMQT_SRC}BoundSettings.cpp
    ${SIMQT_SRC}ColorButton.cpp
    ${SIMQT_SRC}ColorWidget.cpp
    ${SIMQT_SRC}ColorWidgetDelegate.cpp
    ${SIMQT_SRC}ConsoleChannel.cpp
    ${SIMQT_SRC}ConsoleDataModel.cpp
    ${SIMQT_SRC}ConsoleLogger.cpp
    ${SIMQT_SRC}DirectorySelectorWidget.cpp
    ${SIMQT_SRC}DndTreeView.cpp
    ${SIMQT_SRC}DockWidget.cpp
    ${SIMQT_SRC}FileDescriptorReplacement.cpp
    ${SIMQT_SRC}FileDialog.cpp
    ${SIMQT_SRC}FileSelectorWidget.cpp
    ${SIMQT_SRC}FileUtilities.cpp
    ${SIMQT_SRC}FontWidget.cpp
    ${SIMQT_SRC}GanttChartView.cpp
    ${SIMQT_INC}KeySequenceEdit.cpp
    ${SIMQT_SRC}MonospaceItemDelegate.cpp
    ${SIMQT_SRC}MruList.cpp
    ${SIMQT_SRC}PersistentFileLogger.cpp
    ${SIMQT_SRC}QtUtils.cpp
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
    ${SIMQT_SRC}SplashScreen.cpp
    ${SIMQT_SRC}StartupLayoutManager.cpp
    ${SIMQT_SRC}StartupLayoutTask.cpp
    ${SIMQT_SRC}StdStreamConsoleChannel.cpp
    ${SIMQT_SRC}TimeButtons.cpp
    ${SIMQT_SRC}TimeFormatContainer.cpp
    ${SIMQT_SRC}TimeSliderClockBinding.cpp
    ${SIMQT_SRC}TimeWidget.cpp
    ${SIMQT_SRC}Toast.cpp
    ${SIMQT_SRC}UnitsComboBox.cpp
    ${SIMQT_SRC}UnitContext.cpp
    ${SIMQT_SRC}ValueMerge.cpp
    ${SIMQT_SRC}WeightedMenuManager.cpp
    ${SIMQT_SRC}WidgetSettings.cpp
)

# Parts of simQt depend on simData
if(TARGET simData)
    list(APPEND SIMQT_SOURCES
        ${SIMQT_SRC}CategoryDataBreadcrumbs.cpp
        ${SIMQT_SRC}CategoryFilterCounter.cpp
        ${SIMQT_SRC}CategoryFilterWidget.cpp
        ${SIMQT_SRC}CategoryTreeModel.cpp
        ${SIMQT_SRC}DataTableComboBox.cpp
        ${SIMQT_SRC}EntityCategoryFilter.cpp
        ${SIMQT_SRC}EntityFilterLineEdit.cpp
        ${SIMQT_SRC}EntityNameFilter.cpp
        ${SIMQT_SRC}EntityProxyModel.cpp
        ${SIMQT_SRC}EntityStateFilter.cpp
        ${SIMQT_SRC}EntityTreeComposite.cpp
        ${SIMQT_SRC}EntityTreeModel.cpp
        ${SIMQT_SRC}EntityTreeWidget.cpp
        ${SIMQT_SRC}EntityTypeFilter.cpp
        ${SIMQT_SRC}EntityTypeFilterWidget.cpp
        ${SIMQT_SRC}RegExpImpl.cpp
        ${SIMQT_SRC}TimeWidgetClockBinding.cpp
    )
endif()

# Parts of simQt depend on simVis
if(TARGET simVis)
    list(APPEND SIMQT_SOURCES
        ${SIMQT_SRC}CenterEntity.cpp
        ${SIMQT_SRC}EntityLineEdit.cpp
        ${SIMQT_SRC}FillItemModelWithNodeVisitor.cpp
        ${SIMQT_SRC}MapDataModel.cpp
        ${SIMQT_SRC}MultiTouchEventFilter.cpp
        ${SIMQT_SRC}TimestampedLayerManager.cpp
        ${SIMQT_SRC}ViewManagerDataModel.cpp
    )

    # Parts of simQt depend on both simVis and osgQt
    if (TARGET OSGQT)
        list(APPEND SIMQT_SOURCES
            ${SIMQT_SRC}Gl3FormatGuesser.cpp
            ${SIMQT_SRC}ViewWidget.cpp
        )
    endif()

    # Ignore deprecation warnings
    if(MSVC)
        set_source_files_properties(CenterEntity.cpp PROPERTIES COMPILE_FLAGS -wd4996)
    else()
        set_source_files_properties(CenterEntity.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
        set_source_files_properties(Gl3FormatGuesser.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
    endif()
endif()

if(OSG_FOUND)
    list(APPEND SIMQT_SOURCES
        ${SIMQT_INC}ArticulationsEditorWidget.cpp
        ${SIMQT_SRC}ColorGradient.cpp
        ${SIMQT_SRC}ColorGradientWidget.cpp
        ${SIMQT_SRC}QtConversion.cpp
    )
endif()

# Parts of simQt depend on simUtil and simData
if(TARGET simUtil AND TARGET simData)
    list(APPEND SIMQT_SOURCES
        ${SIMQT_SRC}DataTableModel.cpp
    )
endif()

source_group("Source Files" FILES ${SIMQT_SOURCES})

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
    ${SIMQT_INC}SearchLineEditQt4.ui
    ${SIMQT_INC}TimeButtons.ui
)
if(OSG_FOUND)
    list(APPEND SIMQT_FORMS
        ${SIMQT_INC}ArticulationsEditorWidget.ui
        ${SIMQT_INC}ColorGradientWidget.ui
    )
endif()

set(SIMQT_RESOURCES
    ${SIMQT_INC}simQtResources.qrc
)
source_group(Qt FILES ${SIMQT_FORMS} ${SIMQT_RESOURCES})

# Wrap files into generated versions
VSI_QT_WRAP_CPP(Q_HEADERS ${SIMQT_HEADERS_TO_MOC})
VSI_QT_WRAP_UI(Q_FORMS ${SIMQT_FORMS})
VSI_QT_ADD_RESOURCES(Q_RESOURCES ${SIMQT_RESOURCES})

source_group("Generated Files" FILES ${Q_HEADERS} ${Q_FORMS} ${Q_RESOURCES})

set(SIMQT_PROJECT_FILES
    ${SIMQT_HEADERS} ${SIMQT_SOURCES} ${SIMQT_FORMS} ${SIMQT_RESOURCES}
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
    PROJECT_LABEL "simQt"
)
ApplySDKVersion(simQt)
target_include_directories(simQt
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
if(TARGET OSG)
    # QtConversion.h depends on OSG, but not simVis
    target_link_libraries(simQt PUBLIC OSG)
endif()
if(TARGET simData AND TARGET protobuf::libprotobuf)
    get_target_property(_PROTO_INC protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(simQt PUBLIC "$<BUILD_INTERFACE:${_PROTO_INC}>")
endif()
if(TARGET simVis)
    target_include_directories(simQt PUBLIC
        $<BUILD_INTERFACE:${OSG_LIBRARY_INCLUDE_PATH}>
        $<BUILD_INTERFACE:${osgEarth_INCLUDE_DIR}>
    )

    # Gl3FormatGuesser depends on OpenGL libraries
    target_link_libraries(simQt PRIVATE OpenGL::GL)
endif()

# Link against OpenGL if OSGQT is found to support simQt::ViewWidget
target_link_libraries(simQt PUBLIC simCore simNotify)
if(TARGET OSGQT)
    target_link_libraries(simQt PUBLIC OSGQT)
endif()

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
    target_link_libraries(simQt PUBLIC simData)
endif()
if(TARGET simVis)
    target_compile_definitions(simQt PRIVATE HAVE_SIMVIS)
    target_link_libraries(simQt PRIVATE simVis)
    if(TARGET simUtil)
        target_compile_definitions(simQt PRIVATE HAVE_SIMUTIL)
        target_link_libraries(simQt PRIVATE simUtil)
    endif()
elseif(OSG_FOUND)
    target_link_libraries(simQt PRIVATE OSG OPENTHREADS)
endif()

if(TARGET OSGQT)
    VSI_QT_USE_MODULES(simQt LINK_PUBLIC Widgets OpenGL)
else()
    VSI_QT_USE_MODULES(simQt LINK_PUBLIC Widgets)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    # Configure the simQtConfig.cmake file; its dependencies are dynamic
    configure_file(simQtConfig.cmake.in simQtConfig.cmake @ONLY)
    vsi_install_export(simQt ${SIMDIS_SDK_VERSION_STRING} AnyNewerVersion)
endif()

if(AUTOINSTALL_LIBS AND SIMQT_SHARED)
    PostBuildInstallSharedObjects(simQt)
endif()

if(INSTALL_HEADERS)
    # Setup header file installation
    install(FILES ${SIMQT_HEADERS}
        DESTINATION include/simQt
    )
endif()
