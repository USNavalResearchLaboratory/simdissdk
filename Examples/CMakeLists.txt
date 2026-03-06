# SIMDIS SDK examples.

project(SIMDIS_SDK_Examples)

# USE_VCPKG_SIMDISSDK is automatically passed to subdirectories

if(USE_VCPKG_SIMDISSDK)
    message(STATUS "Using vcpkg's simdissdk in Examples")
    # Use the individual CMake config files for each library
    find_package(simCore CONFIG REQUIRED)
    find_package(simData CONFIG REQUIRED)
    find_package(simNotify CONFIG REQUIRED)
    find_package(simQt CONFIG REQUIRED)
    find_package(simUtil CONFIG REQUIRED)
    find_package(simVis CONFIG REQUIRED)
    # Set up aliases for the library na
    add_library(simCore ALIAS VSI::simCore)
    add_library(simData ALIAS VSI::simData)
    add_library(simNotify ALIAS VSI::simNotify)
    add_library(simQt ALIAS VSI::simQt)
    add_library(simUtil ALIAS VSI::simUtil)
    add_library(simVis ALIAS VSI::simVis)
endif()

# Set up OSGEARTH plugin directory
if(WIN32)
    # Set OSG plugin paths
    # For vcpkg, plugins are in plugins directory, not lib directory
    # Note: OSG will automatically look for osgPlugins-<version> subdirectory
    if(USE_VCPKG_SIMDISSDK AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        SET(OSG_PLUGINS_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/plugins")
        SET(OSG_PLUGINS_DIR_DEBUG "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/plugins")
        message(STATUS "OSG_PLUGINS_DIR=${OSG_PLUGINS_DIR}")
        message(STATUS "OSG_PLUGINS_DIR_DEBUG=${OSG_PLUGINS_DIR_DEBUG}")
    endif()
    
    # Function to set up OSGEARTH plugin directory for each example
    function(setup_osgearth_plugin_dir TARGET_NAME)
        if(OSG_PLUGINS_DIR)
            # Set Visual Studio debugger environment variables
            # VS_DEBUGGER_ENVIRONMENT: Sets the 'Environment' property for debugging
            # Use debug paths for debug builds if available
            
            # Check if debug directories exist
            SET(USE_DEBUG_PATHS FALSE)
            IF(OSG_PLUGINS_DIR_DEBUG)
                IF(EXISTS "${OSG_PLUGINS_DIR_DEBUG}")
                    SET(USE_DEBUG_PATHS TRUE)
                ENDIF()
            ENDIF()
            
            IF(USE_DEBUG_PATHS)
                # For vcpkg, use debug paths (which contain the 'd' suffixed DLLs)
                # Use semicolon to separate multiple paths in PATH variable
                SET(OSG_ENVIRONMENT "OSG_LIBRARY_PATH=${OSG_PLUGINS_DIR_DEBUG};${OSG_PLUGINS_DIR}")
                message(STATUS "Example ${TARGET_NAME}: Using Debug plugin paths")
            ELSE()
                SET(OSG_ENVIRONMENT "OSG_LIBRARY_PATH=${OSG_PLUGINS_DIR}")
                message(STATUS "Example ${TARGET_NAME}: Using Release plugin paths")
            ENDIF()
            
            SET_PROPERTY(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_ENVIRONMENT "${OSG_ENVIRONMENT}")
            message(STATUS "Example ${TARGET_NAME}: VS_DEBUGGER_ENVIRONMENT=${OSG_ENVIRONMENT}")
        endif()
    endfunction()
endif()

add_subdirectory(imgui)

add_subdirectory(AnimatedLine)
add_subdirectory(AntennaPattern)
add_subdirectory(ArticulatedModel)
add_subdirectory(AsyncModelLoading)
add_subdirectory(BasicViewer)
add_subdirectory(BasicViewerText)
add_subdirectory(CentroidEyePosition)
add_subdirectory(CustomRenderingTest)
add_subdirectory(FragmentEffectTest)
add_subdirectory(GeoFencing)
add_subdirectory(GOGAttachments)
add_subdirectory(GOGReader)
add_subdirectory(HudPositionManager)
add_subdirectory(ImageIcons)
add_subdirectory(MapScale)
add_subdirectory(MassiveData)
add_subdirectory(ObserverMaker)
add_subdirectory(Ocean)
add_subdirectory(Periscope)
add_subdirectory(Picking)
add_subdirectory(PlatformSymbology)
add_subdirectory(Projectors)
add_subdirectory(RadialLOS)
add_subdirectory(RangeTool)
add_subdirectory(RCS)
add_subdirectory(RFProp)
add_subdirectory(RocketBurn)
add_subdirectory(SkyModel)
add_subdirectory(SimpleServer)
add_subdirectory(TimestampedLayer)
add_subdirectory(VelocityLayerTest)

if(TARGET SQLite::SQLite3)
    add_subdirectory(DBReader)
    add_subdirectory(LoadEarthFile)
    add_subdirectory(Overhead)
endif()

if(Qt5Core_FOUND OR Qt6Core_FOUND)
    add_subdirectory(Qt)
    add_subdirectory(QtActionItemModelTest)
    add_subdirectory(QtCategoryFilterTest)
    add_subdirectory(QtColorWidgetTest)
    add_subdirectory(QtConsoleDataModel)
    add_subdirectory(QtDataTableViewTest)
    add_subdirectory(QtDockableViews)
    add_subdirectory(QtDragAndDrop)
    add_subdirectory(QtEntityLineEditTest)
    add_subdirectory(QtEntityViewTest)
    add_subdirectory(QtFileSelectorTest)
    add_subdirectory(QtHudLabelBins)
    add_subdirectory(QtThreadExample)
    add_subdirectory(QtTimeButtons)
    add_subdirectory(QtTimeWidgetTest)
    add_subdirectory(QtTouchViewer)
    add_subdirectory(QtUnitsWidgetTest)
    add_subdirectory(QtViewManagerDataModel)
    add_subdirectory(QtViewManagerTest)
    add_subdirectory(QtWidgetNodes)
endif()


add_subdirectory(AngleTest)
add_subdirectory(BeamTest)
add_subdirectory(GateTest)
add_subdirectory(EciTrackHistoryTest)
add_subdirectory(LocatorTest)
add_subdirectory(PlanetariumViewTest)
add_subdirectory(PlatformAzimElevViewTest)
add_subdirectory(LOBTest)
add_subdirectory(TrackHistoryTest)
add_subdirectory(ViewManagerTest)
add_subdirectory(ASIViewer)
add_subdirectory(LocalGrid)
