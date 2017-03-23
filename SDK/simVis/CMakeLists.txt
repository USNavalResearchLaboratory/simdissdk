if(NOT TARGET SIMDIS_SDKData OR NOT OSG_FOUND OR NOT OSGEARTH_FOUND)
    message(STATUS "Skipping simVis because of missing dependencies.")
    return()
endif()

# ----- simVis --------------------------------------------------------

set(VIS_INC)
set(VIS_SRC)

set(SHADER_FILES
    AlphaColorFilter.frag.glsl
    AreaHighlight.vert.glsl
    AreaHighlight.frag.glsl
    BathymetryGenerator.vert.glsl
    GlowHighlight.frag.glsl
    OverrideColor.frag.glsl
    OverheadMode.vert.glsl
    PlatformAzimElevViewTool.vert.glsl
    Projector.vert.glsl
    Projector.frag.glsl
    RF.LossToColor.Default.lib.glsl
    RF.LossToColor.Threshold.lib.glsl
    RF.Texture.vert.glsl
    RF.Texture.frag.glsl
    RF.Vertex.vert.glsl
    RF.Vertex.frag.glsl
    SetClipVertex.vert.glsl
    TrackHistory.vert.glsl
    TrackHistory.frag.glsl
)

set(VIS_HEADERS_CORE
    ${VIS_INC}AlphaColorFilter.h
    ${VIS_INC}AnimatedLine.h
    ${VIS_INC}Antenna.h
    ${VIS_INC}AreaHighlight.h
    ${VIS_INC}AxisVector.h
    ${VIS_INC}BathymetryGenerator.h
    ${VIS_INC}Beam.h
    ${VIS_INC}ClassificationBanner.h
    ${VIS_INC}ClockOptions.h
    ${VIS_INC}Compass.h
    ${VIS_INC}Constants.h
    ${VIS_INC}CylinderGeode.h
    ${VIS_INC}CylinderStorage.h
    ${VIS_INC}DBOptions.h
    ${VIS_INC}DynamicScaleTransform.h
    ${VIS_INC}EarthManipulator.h
    ${VIS_INC}ElevationQueryProxy.h
    ${VIS_INC}Entity.h
    ${VIS_INC}EntityFamily.h
    ${VIS_INC}EntityLabel.h
    ${VIS_INC}EphemerisVector.h
    ${VIS_INC}EyePositionManager.h
    ${VIS_INC}Gate.h
    ${VIS_INC}GeoFence.h
    ${VIS_INC}GlowHighlight.h
    ${VIS_INC}InsetViewEventHandler.h
    ${VIS_INC}LabelContentManager.h
    ${VIS_INC}Laser.h
    ${VIS_INC}LightAmbient.h
    ${VIS_INC}LightDiffuse.h
    ${VIS_INC}LobGroup.h
    ${VIS_INC}LocalGrid.h
    ${VIS_INC}Locator.h
    ${VIS_INC}NavigationModes.h
    ${VIS_INC}ObjectSelectionHandler.h
    ${VIS_INC}OverheadMode.h
    ${VIS_INC}OverrideColor.h
    ${VIS_INC}PlanetariumViewTool.h
    ${VIS_INC}Platform.h
    ${VIS_INC}PlatformAzimElevViewTool.h
    ${VIS_INC}PlatformFilter.h
    ${VIS_INC}PlatformInertialTransform.h
    ${VIS_INC}PlatformModel.h
    ${VIS_INC}Popup.h
    ${VIS_INC}Projector.h
    ${VIS_INC}ProjectorManager.h
    ${VIS_INC}RadialLOS.h
    ${VIS_INC}RadialLOSNode.h
    ${VIS_INC}RangeTool.h
    ${VIS_INC}RCS.h
    ${VIS_INC}Registry.h
    ${VIS_INC}RocketBurn.h
    ${VIS_INC}RocketBurnStorage.h
    ${VIS_INC}Scenario.h
    ${VIS_INC}ScenarioDataStoreAdapter.h
    ${VIS_INC}SceneManager.h
    ${VIS_INC}SphericalVolume.h
    ${VIS_INC}TargetDelegation.h
    ${VIS_INC}Text.h
    ${VIS_INC}Tool.h
    ${VIS_INC}TrackChunkNode.h
    ${VIS_INC}TrackHistory.h
    ${VIS_INC}Types.h
    ${VIS_INC}Utils.h
    ${VIS_INC}VaporTrail.h
    ${VIS_INC}VaporTrailStorage.h
    ${VIS_INC}VelocityVector.h
    ${VIS_INC}View.h
    ${VIS_INC}Viewer.h
    ${VIS_INC}ViewManager.h
    ${VIS_INC}ViewManagerLogDbAdapter.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/simVis/osgEarthVersion.h
)

set(VIS_HEADERS_RFPROP
    ${VIS_INC}RFProp/ArepsLoader.h
    ${VIS_INC}RFProp/BearingProfileMap.h
    ${VIS_INC}RFProp/ColorProvider.h
    ${VIS_INC}RFProp/CompositeColorProvider.h
    ${VIS_INC}RFProp/CompositeProfileProvider.h
    ${VIS_INC}RFProp/FunctionalProfileDataProvider.h
    ${VIS_INC}RFProp/GradientColorProvider.h
    ${VIS_INC}RFProp/LUTProfileDataProvider.h
    ${VIS_INC}RFProp/LUT1ProfileDataProvider.h
    ${VIS_INC}RFProp/OneWayPowerDataProvider.h
    ${VIS_INC}RFProp/PODProfileDataProvider.h
    ${VIS_INC}RFProp/Profile.h
    ${VIS_INC}RFProp/ProfileDataProvider.h
    ${VIS_INC}RFProp/ProfileManager.h
    ${VIS_INC}RFProp/RadarParameters.h
    ${VIS_INC}RFProp/RFPropagationFacade.h
    ${VIS_INC}RFProp/RFPropagationManager.h
    ${VIS_INC}RFProp/SNRDataProvider.h
    ${VIS_INC}RFProp/ThresholdColorProvider.h
    ${VIS_INC}RFProp/TwoWayPowerDataProvider.h
)

set(VIS_HEADERS_GOG
    ${VIS_INC}GOG/Annotation.h
    ${VIS_INC}GOG/Arc.h
    ${VIS_INC}GOG/Circle.h
    ${VIS_INC}GOG/Cylinder.h
    ${VIS_INC}GOG/Ellipse.h
    ${VIS_INC}GOG/Ellipsoid.h
    ${VIS_INC}GOG/ErrorHandler.h
    ${VIS_INC}GOG/GOG.h
    ${VIS_INC}GOG/GOGNode.h
    ${VIS_INC}GOG/GogNodeInterface.h
    ${VIS_INC}GOG/GOGRegistry.h
    ${VIS_INC}GOG/Hemisphere.h
    ${VIS_INC}GOG/HostedLocalGeometryNode.h
    ${VIS_INC}GOG/LatLonAltBox.h
    ${VIS_INC}GOG/Line.h
    ${VIS_INC}GOG/LineSegs.h
    ${VIS_INC}GOG/Parser.h
    ${VIS_INC}GOG/Points.h
    ${VIS_INC}GOG/Polygon.h
    ${VIS_INC}GOG/Sphere.h
    ${VIS_INC}GOG/Utils.h
)

set(VIS_SOURCES_CORE
    ${VIS_SRC}AnimatedLine.cpp
    ${VIS_SRC}Antenna.cpp
    ${VIS_SRC}AreaHighlight.cpp
    ${VIS_SRC}AxisVector.cpp
    ${VIS_SRC}BathymetryGenerator.cpp
    ${VIS_SRC}Beam.cpp
    ${VIS_SRC}ClassificationBanner.cpp
    ${VIS_SRC}ClockOptions.cpp
    ${VIS_SRC}Compass.cpp
    ${VIS_SRC}CylinderGeode.cpp
    ${VIS_SRC}CylinderStorage.cpp
    ${VIS_SRC}DynamicScaleTransform.cpp
    ${VIS_SRC}EarthManipulator.cpp
    ${VIS_SRC}ElevationQueryProxy.cpp
    ${VIS_SRC}Entity.cpp
    ${VIS_SRC}EntityFamily.cpp
    ${VIS_SRC}EntityLabel.cpp
    ${VIS_SRC}EphemerisVector.cpp
    ${VIS_SRC}Gate.cpp
    ${VIS_SRC}GeoFence.cpp
    ${VIS_SRC}GlowHighlight.cpp
    ${VIS_SRC}Laser.cpp
    ${VIS_SRC}LightAmbient.cpp
    ${VIS_SRC}LightDiffuse.cpp
    ${VIS_SRC}LobGroup.cpp
    ${VIS_SRC}LocalGrid.cpp
    ${VIS_SRC}Locator.cpp
    ${VIS_SRC}OverheadMode.cpp
    ${VIS_SRC}OverrideColor.cpp
    ${VIS_SRC}PlanetariumViewTool.cpp
    ${VIS_SRC}Platform.cpp
    ${VIS_SRC}PlatformAzimElevViewTool.cpp
    ${VIS_SRC}PlatformFilter.cpp
    ${VIS_SRC}PlatformInertialTransform.cpp
    ${VIS_SRC}PlatformModel.cpp
    ${VIS_SRC}Popup.cpp
    ${VIS_SRC}Projector.cpp
    ${VIS_SRC}ProjectorManager.cpp
    ${VIS_SRC}RadialLOS.cpp
    ${VIS_SRC}RadialLOSNode.cpp
    ${VIS_SRC}RangeTool.cpp
    ${VIS_SRC}RCS.cpp
    ${VIS_SRC}Registry.cpp
    ${VIS_SRC}RocketBurn.cpp
    ${VIS_SRC}RocketBurnStorage.cpp
    ${VIS_SRC}Scenario.cpp
    ${VIS_SRC}ScenarioDataStoreAdapter.cpp
    ${VIS_SRC}SceneManager.cpp
    ${VIS_SRC}SphericalVolume.cpp
    ${VIS_SRC}TargetDelegation.cpp
    ${VIS_SRC}Text.cpp
    ${VIS_SRC}TrackChunkNode.cpp
    ${VIS_SRC}TrackHistory.cpp
    ${VIS_SRC}Utils.cpp
    ${VIS_SRC}AlphaColorFilter.cpp
    ${VIS_SRC}InsetViewEventHandler.cpp
    ${VIS_SRC}View.cpp
    ${VIS_SRC}NavigationModes.cpp
    ${VIS_SRC}ObjectSelectionHandler.cpp
    ${VIS_SRC}VaporTrail.cpp
    ${VIS_SRC}VaporTrailStorage.cpp
    ${VIS_SRC}VelocityVector.cpp
    ${VIS_SRC}Viewer.cpp
    ${VIS_SRC}ViewManager.cpp
    ${VIS_SRC}ViewManagerLogDbAdapter.cpp
)

set(VIS_SOURCES_RFPROP
    ${VIS_SRC}RFProp/ArepsLoader.cpp
    ${VIS_SRC}RFProp/BearingProfileMap.cpp
    ${VIS_SRC}RFProp/CompositeColorProvider.cpp
    ${VIS_SRC}RFProp/CompositeProfileProvider.cpp
    ${VIS_SRC}RFProp/FunctionalProfileDataProvider.cpp
    ${VIS_SRC}RFProp/GradientColorProvider.cpp
    ${VIS_SRC}RFProp/LUTProfileDataProvider.cpp
    ${VIS_SRC}RFProp/LUT1ProfileDataProvider.cpp
    ${VIS_SRC}RFProp/OneWayPowerDataProvider.cpp
    ${VIS_SRC}RFProp/PODProfileDataProvider.cpp
    ${VIS_SRC}RFProp/Profile.cpp
    ${VIS_SRC}RFProp/ProfileManager.cpp
    ${VIS_SRC}RFProp/RFPropagationFacade.cpp
    ${VIS_SRC}RFProp/SNRDataProvider.cpp
    ${VIS_SRC}RFProp/ThresholdColorProvider.cpp
    ${VIS_SRC}RFProp/TwoWayPowerDataProvider.cpp
)

set(VIS_SOURCES_GOG
    ${VIS_SRC}GOG/Annotation.cpp
    ${VIS_SRC}GOG/Arc.cpp
    ${VIS_SRC}GOG/Circle.cpp
    ${VIS_SRC}GOG/Cylinder.cpp
    ${VIS_SRC}GOG/Ellipse.cpp
    ${VIS_SRC}GOG/Ellipsoid.cpp
    ${VIS_SRC}GOG/GOG.cpp
    ${VIS_SRC}GOG/GOGNode.cpp
    ${VIS_SRC}GOG/GogNodeInterface.cpp
    ${VIS_SRC}GOG/GOGRegistry.cpp
    ${VIS_SRC}GOG/Hemisphere.cpp
    ${VIS_SRC}GOG/LatLonAltBox.cpp
    ${VIS_SRC}GOG/Line.cpp
    ${VIS_SRC}GOG/LineSegs.cpp
    ${VIS_SRC}GOG/Parser.cpp
    ${VIS_SRC}GOG/Points.cpp
    ${VIS_SRC}GOG/Polygon.cpp
    ${VIS_SRC}GOG/Sphere.cpp
    ${VIS_SRC}GOG/Utils.cpp
)

set(VIS_ALL_HEADER
    ${VIS_INC}../simVis.h
)

# Detect the osgEarth version
set(SDK_OSGEARTH_DEVEL 0)
if(OSGEARTH_TRITON_SUPPORT STREQUAL NODEKIT)
    set(HAVE_TRITON_NODEKIT 1)
endif()
if(OSGEARTH_SILVERLINING_SUPPORT STREQUAL NODEKIT)
    set(HAVE_SILVERLINING_NODEKIT 1)
endif()
include(CheckOSGEarthMethodExists)
# Newer than 1.6.0 release removes tileSize() from TileSource
check_osgearth_method_exists(HAVE_OSGEARTH_TILESOURCE_TILESIZE "osgEarth/TileSource" "tileSize")
if(NOT HAVE_OSGEARTH_TILESOURCE_TILESIZE)
    set(SDK_OSGEARTH_DEVEL 1)
    # Next anticipated version is 1.7.0
    set(SDK_OSGEARTH_VERSION_MAJOR 1)
    set(SDK_OSGEARTH_VERSION_MINOR 7)
    set(SDK_OSGEARTH_VERSION_PATCH 0)
else()
    # Version 1.6.0 changed Map's setName to setMapName
    check_osgearth_method_exists(HAVE_OSGEARTH_MAP_SETMAPNAME "osgEarth/Map" "setMapName")
    if(HAVE_OSGEARTH_MAP_SETMAPNAME)
        set(SDK_OSGEARTH_VERSION_MAJOR 1)
        set(SDK_OSGEARTH_VERSION_MINOR 6)
        set(SDK_OSGEARTH_VERSION_PATCH 0)
    else()
        # Only support back to osgearth-simdis-sdk-1.5.0 (osgEarth 2.8)
        set(SDK_OSGEARTH_VERSION_MAJOR 1)
        set(SDK_OSGEARTH_VERSION_MINOR 5)
        set(SDK_OSGEARTH_VERSION_PATCH 0)
    endif()
endif()

# Configure the osgEarth versions file and print out the information to CMake
if(VERBOSE)
    if(SDK_OSGEARTH_DEVEL)
        message(STATUS "Detected development osgEarth version.")
    else()
        message(STATUS "Detected osgEarth version osgearth-simdis-sdk-${SDK_OSGEARTH_VERSION_MAJOR}.${SDK_OSGEARTH_VERSION_MINOR}")
    endif()
endif()
configure_file(${VIS_INC}osgEarthVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/simVis/osgEarthVersion.h @ONLY)

# Note that shaders do not go into simVis.h and header/cpp don't get installed
set(VIS_SHADER_FILES
    ${SHADER_FILES}
    ${VIS_INC}Shaders.h
    ${VIS_SRC}Shaders.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/Shaders.cpp
)
include(ConfigureShaders)
configure_shaders(Shaders.cpp.in Shaders.cpp ${SHADER_FILES})
source_group(Shaders FILES ${VIS_SHADER_FILES})

source_group(Headers FILES ${VIS_HEADERS_CORE})
source_group(Sources FILES ${VIS_SOURCES_CORE})

source_group(Headers\\RFProp FILES ${VIS_HEADERS_RFPROP})
source_group(Sources\\RFProp FILES ${VIS_SOURCES_RFPROP})

source_group(Headers\\GOG FILES ${VIS_HEADERS_GOG})
source_group(Sources\\GOG FILES ${VIS_SOURCES_GOG})

source_group("" FILES ${VIS_ALL_HEADER})

set(VIS_HEADERS ${VIS_HEADERS_CORE} ${VIS_HEADERS_RFPROP} ${VIS_HEADERS_GOG})
set(VIS_SOURCES ${VIS_SOURCES_CORE} ${VIS_SOURCES_RFPROP} ${VIS_SOURCES_GOG} ${VIS_SHADER_FILES})

# ----------------------------------------------------------------------

set(VIS_PROJECT_FILES
    ${VIS_HEADERS} ${VIS_SOURCES} ${VIS_ALL_HEADER}
)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMVIS_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(SIMDIS_SDKVis ${STATIC_OR_SHARED} ${VIS_PROJECT_FILES})
set_target_properties(SIMDIS_SDKVis PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "SIMDIS SDK - Vis"
)
target_include_directories(SIMDIS_SDKVis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(SIMDIS_SDKVis PUBLIC SIMDIS_SDKData SIMDIS_SDKCore SIMDIS_SDKNotify
    ${OSG_ALL_LIBDEPENDENCIES} ${OSGEARTH_ALL_LIBDEPENDENCIES})
if(SIMVIS_SHARED)
    target_compile_definitions(SIMDIS_SDKVis PRIVATE SIMDIS_SDKVis_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(SIMDIS_SDKVis PUBLIC SIMDIS_SDKVis_LIB_EXPORT_STATIC)
endif()


# FFP in LightDiffuse and LightAmbient requires linking to OpenGL
find_package(OpenGL)
if(OPENGL_FOUND)
    target_link_libraries(SIMDIS_SDKVis PRIVATE ${OPENGL_LIBRARIES})
    target_compile_definitions(SIMDIS_SDKVis PRIVATE SIMVIS_GL_FIXED_FUNCTION_AVAILABLE)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    set_target_properties(SIMDIS_SDKVis PROPERTIES TARGET_EXPORT_NAME "SIMDIS_SDK")
    vsi_install_target(SIMDIS_SDKVis SDK_Library)
endif()

if(INSTALL_SIMDIS_SDK_SHADERS)
    if(NOT SIMDIS_SDK_SHADERS_INSTALL_DIR)
        set(SIMDIS_SDK_SHADERS_INSTALL_DIR data/shaders)
    endif()
    install(FILES ${SHADER_FILES}
        COMPONENT SDK_Library
        DESTINATION ${SIMDIS_SDK_SHADERS_INSTALL_DIR}
    )
endif()

if(AUTOINSTALL_LIBS AND SIMVIS_SHARED)
    PostBuildInstallSdkSharedObjects(SIMDIS_SDKVis)
endif()

# Setup header file installation
if(INSTALL_HEADERS)
    install(FILES ${VIS_HEADERS_CORE}
        DESTINATION include/simVis)
    install(FILES ${VIS_HEADERS_RFPROP}
        DESTINATION include/simVis/RFProp)
    install(FILES ${VIS_HEADERS_GOG}
        DESTINATION include/simVis/GOG)
    install(FILES ${VIS_ALL_HEADER}
        DESTINATION include)
endif()
