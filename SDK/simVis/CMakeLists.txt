if(NOT TARGET simData OR NOT OSG_FOUND OR NOT TARGET osgEarth::osgEarth)
    message(STATUS "Skipping simVis because of missing dependencies.")
    return()
endif()

# Variables used in the simVisConfig.cmake generation
set(SIMVIS_HAVE_SQLITE OFF)

# ----- simVis --------------------------------------------------------

set(VIS_INC)
set(VIS_SRC)

set(SHADER_FILES
    AlphaColorFilter.frag.glsl
    AlphaTest.frag.glsl
    AreaHighlight.vert.glsl
    AreaHighlight.frag.glsl
    BathymetryGenerator.vert.glsl
    BeamPulse.vert.glsl
    BeamPulse.frag.glsl
    DisableDepthOnAlpha.frag.glsl
    FragmentEffect.glsl
    Flashing.frag.glsl
    LineDrawable.glsl
    OverrideColor.frag.glsl
    OverheadMode.vert.glsl
    Picker.vert.glsl
    Picker.frag.glsl
    PlatformAzimElevViewTool.vert.glsl
    PlanetariumTexture.glsl
    PolygonStipple.frag.glsl
    Projector.vert.glsl
    Projector.frag.glsl
    ProjectorOnEntity.glsl
    RF.LossToColor.Default.lib.glsl
    RF.LossToColor.Threshold.lib.glsl
    RF.Texture.vert.glsl
    RF.Texture.frag.glsl
    RF.Vertex.vert.glsl
    RF.Vertex.frag.glsl
    RocketBurn.glsl
    SetClipVertex.vert.glsl
    TrackHistory.vert.glsl
    TrackHistory.frag.glsl
)

set(VIS_HEADERS_CORE
    ${VIS_INC}AlphaColorFilter.h
    ${VIS_INC}AlphaTest.h
    ${VIS_INC}AnimatedLine.h
    ${VIS_INC}Antenna.h
    ${VIS_INC}AreaHighlight.h
    ${VIS_INC}AveragePositionNode.h
    ${VIS_INC}AxisVector.h
    ${VIS_INC}BathymetryGenerator.h
    ${VIS_INC}Beam.h
    ${VIS_INC}BeamPulse.h
    ${VIS_INC}BillboardAutoTransform.h
    ${VIS_INC}BoxGraphic.h
    ${VIS_INC}BoxZoomMouseHandler.h
    ${VIS_INC}BrightnessContrastColorFilter.h
    ${VIS_INC}CentroidManager.h
    ${VIS_INC}ChromaKeyColorFilter.h
    ${VIS_INC}ClassificationBanner.h
    ${VIS_INC}ClockOptions.h
    ${VIS_INC}Compass.h
    ${VIS_INC}Constants.h
    ${VIS_INC}CustomRendering.h
    ${VIS_INC}CylinderGeode.h
    ${VIS_INC}CylinderStorage.h
    ${VIS_INC}DisableDepthOnAlpha.h
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
    ${VIS_INC}Gl3Utils.h
    ${VIS_INC}GlDebugMessage.h
    ${VIS_INC}GradientShader.h
    ${VIS_INC}Headless.h
    ${VIS_INC}InsetViewEventHandler.h
    ${VIS_INC}LabelContentManager.h
    ${VIS_INC}Laser.h
    ${VIS_INC}LobGroup.h
    ${VIS_INC}LocalGrid.h
    ${VIS_INC}Locator.h
    ${VIS_INC}LocatorNode.h
    ${VIS_INC}Measurement.h
    ${VIS_INC}ModelCache.h
    ${VIS_INC}FragmentEffect.h
    ${VIS_INC}ModKeyHandler.h
    ${VIS_INC}NavigationModes.h
    ${VIS_INC}ObjectSelectionHandler.h
    ${VIS_INC}OverheadMode.h
    ${VIS_INC}OverrideColor.h
    ${VIS_INC}LayerRefreshCallback.h
    ${VIS_INC}Picker.h
    ${VIS_INC}PlanetariumViewTool.h
    ${VIS_INC}Platform.h
    ${VIS_INC}PlatformAzimElevViewTool.h
    ${VIS_INC}PlatformFilter.h
    ${VIS_INC}PlatformIconFactory.h
    ${VIS_INC}PlatformInertialTransform.h
    ${VIS_INC}PlatformModel.h
    ${VIS_INC}PolygonStipple.h
    ${VIS_INC}Popup.h
    ${VIS_INC}Projector.h
    ${VIS_INC}ProjectorManager.h
    ${VIS_INC}RadialLOS.h
    ${VIS_INC}RadialLOSNode.h
    ${VIS_INC}RangeTool.h
    ${VIS_INC}RangeToolState.h
    ${VIS_INC}RCS.h
    ${VIS_INC}Registry.h
    ${VIS_INC}RocketBurn.h
    ${VIS_INC}RocketBurnStorage.h
    ${VIS_INC}Scenario.h
    ${VIS_INC}ScenarioDataStoreAdapter.h
    ${VIS_INC}SceneManager.h
    ${VIS_INC}SimdisMeasurement.h
    ${VIS_INC}SimdisRangeToolState.h
    ${VIS_INC}SphericalVolume.h
    ${VIS_INC}TargetDelegation.h
    ${VIS_INC}Text.h
    ${VIS_INC}TimeTicks.h
    ${VIS_INC}TimeTicksChunk.h
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

set(VIS_HEADERS_DB
    ${VIS_INC}DBFormat.h
    ${VIS_INC}DBOptions.h
)

set(VIS_HEADERS_DB_PRIVATE
    ${VIS_INC}DB/QSCommon.h
    ${VIS_INC}DB/QSError.h
    ${VIS_INC}DB/QSNodeID96.h
    ${VIS_INC}DB/QSPosXYExtents.h
    ${VIS_INC}DB/SQLiteDataBaseReadUtil.h
    ${VIS_INC}DB/swapbytes.h
)

set(VIS_HEADERS_RFPROP
    ${VIS_INC}RFProp/ArepsLoader.h
    ${VIS_INC}RFProp/BearingProfileMap.h
    ${VIS_INC}RFProp/ColorProvider.h
    ${VIS_INC}RFProp/CompositeColorProvider.h
    ${VIS_INC}RFProp/CompositeProfileProvider.h
    ${VIS_INC}RFProp/FallbackDataHelper.h
    ${VIS_INC}RFProp/FunctionalProfileDataProvider.h
    ${VIS_INC}RFProp/GradientColorProvider.h
    ${VIS_INC}RFProp/LUTProfileDataProvider.h
    ${VIS_INC}RFProp/LUT1ProfileDataProvider.h
    ${VIS_INC}RFProp/OneWayPowerDataProvider.h
    ${VIS_INC}RFProp/PODProfileDataProvider.h
    ${VIS_INC}RFProp/PPFDataProvider.h
    ${VIS_INC}RFProp/Profile.h
    ${VIS_INC}RFProp/ProfileContext.h
    ${VIS_INC}RFProp/ProfileDataProvider.h
    ${VIS_INC}RFProp/ProfileManager.h
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
    ${VIS_INC}GOG/Cone.h
    ${VIS_INC}GOG/Cylinder.h
    ${VIS_INC}GOG/Ellipse.h
    ${VIS_INC}GOG/Ellipsoid.h
    ${VIS_INC}GOG/ErrorHandler.h
    ${VIS_INC}GOG/GOGNode.h
    ${VIS_INC}GOG/GogNodeInterface.h
    ${VIS_INC}GOG/Hemisphere.h
    ${VIS_INC}GOG/HostedLocalGeometryNode.h
    ${VIS_INC}GOG/ImageOverlay.h
    ${VIS_INC}GOG/LatLonAltBox.h
    ${VIS_INC}GOG/Line.h
    ${VIS_INC}GOG/LineSegs.h
    ${VIS_INC}GOG/Loader.h
    ${VIS_INC}GOG/LoaderUtils.h
    ${VIS_INC}GOG/Orbit.h
    ${VIS_INC}GOG/ParsedShape.h
    ${VIS_INC}GOG/Points.h
    ${VIS_INC}GOG/Polygon.h
    ${VIS_INC}GOG/Sphere.h
    ${VIS_INC}GOG/Utils.h
)

set(VIS_SOURCES_CORE
    ${VIS_SRC}AlphaColorFilter.cpp
    ${VIS_SRC}AlphaTest.cpp
    ${VIS_SRC}AnimatedLine.cpp
    ${VIS_SRC}Antenna.cpp
    ${VIS_SRC}AreaHighlight.cpp
    ${VIS_SRC}AveragePositionNode.cpp
    ${VIS_SRC}AxisVector.cpp
    ${VIS_SRC}BathymetryGenerator.cpp
    ${VIS_SRC}Beam.cpp
    ${VIS_SRC}BeamPulse.cpp
    ${VIS_SRC}BillboardAutoTransform.cpp
    ${VIS_SRC}BoxGraphic.cpp
    ${VIS_SRC}BoxZoomMouseHandler.cpp
    ${VIS_SRC}BrightnessContrastColorFilter.cpp
    ${VIS_SRC}CentroidManager.cpp
    ${VIS_SRC}ChromaKeyColorFilter.cpp
    ${VIS_SRC}ClassificationBanner.cpp
    ${VIS_SRC}ClockOptions.cpp
    ${VIS_SRC}Compass.cpp
    ${VIS_SRC}CustomRendering.cpp
    ${VIS_SRC}CylinderGeode.cpp
    ${VIS_SRC}CylinderStorage.cpp
    ${VIS_SRC}DisableDepthOnAlpha.cpp
    ${VIS_SRC}DynamicScaleTransform.cpp
    ${VIS_SRC}EarthManipulator.cpp
    ${VIS_SRC}ElevationQueryProxy.cpp
    ${VIS_SRC}Entity.cpp
    ${VIS_SRC}EntityFamily.cpp
    ${VIS_SRC}EntityLabel.cpp
    ${VIS_SRC}EphemerisVector.cpp
    ${VIS_SRC}Gate.cpp
    ${VIS_SRC}GeoFence.cpp
    ${VIS_INC}GlDebugMessage.cpp
    ${VIS_SRC}GradientShader.cpp
    ${VIS_SRC}Headless.cpp
    ${VIS_SRC}InsetViewEventHandler.cpp
    ${VIS_SRC}Laser.cpp
    ${VIS_SRC}LayerRefreshCallback.cpp
    ${VIS_SRC}LobGroup.cpp
    ${VIS_SRC}LocalGrid.cpp
    ${VIS_SRC}Locator.cpp
    ${VIS_SRC}LocatorNode.cpp
    ${VIS_SRC}Measurement.cpp
    ${VIS_SRC}ModelCache.cpp
    ${VIS_SRC}FragmentEffect.cpp
    ${VIS_SRC}ModKeyHandler.cpp
    ${VIS_SRC}NavigationModes.cpp
    ${VIS_SRC}ObjectSelectionHandler.cpp
    ${VIS_SRC}OverheadMode.cpp
    ${VIS_SRC}OverrideColor.cpp
    ${VIS_SRC}Picker.cpp
    ${VIS_SRC}PlanetariumViewTool.cpp
    ${VIS_SRC}Platform.cpp
    ${VIS_SRC}PlatformAzimElevViewTool.cpp
    ${VIS_SRC}PlatformFilter.cpp
    ${VIS_SRC}PlatformIconFactory.cpp
    ${VIS_SRC}PlatformInertialTransform.cpp
    ${VIS_SRC}PlatformModel.cpp
    ${VIS_SRC}PolygonStipple.cpp
    ${VIS_SRC}Popup.cpp
    ${VIS_SRC}Projector.cpp
    ${VIS_SRC}ProjectorManager.cpp
    ${VIS_SRC}RadialLOS.cpp
    ${VIS_SRC}RadialLOSNode.cpp
    ${VIS_SRC}RangeTool.cpp
    ${VIS_SRC}RangeToolState.cpp
    ${VIS_SRC}RCS.cpp
    ${VIS_SRC}Registry.cpp
    ${VIS_SRC}RocketBurn.cpp
    ${VIS_SRC}RocketBurnStorage.cpp
    ${VIS_SRC}Scenario.cpp
    ${VIS_SRC}ScenarioDataStoreAdapter.cpp
    ${VIS_SRC}SceneManager.cpp
    ${VIS_SRC}SimdisMeasurement.cpp
    ${VIS_SRC}SimdisRangeToolState.cpp
    ${VIS_SRC}SphericalVolume.cpp
    ${VIS_SRC}TargetDelegation.cpp
    ${VIS_SRC}Text.cpp
    ${VIS_SRC}TimeTicks.cpp
    ${VIS_SRC}TimeTicksChunk.cpp
    ${VIS_SRC}TrackChunkNode.cpp
    ${VIS_SRC}TrackHistory.cpp
    ${VIS_SRC}Utils.cpp
    ${VIS_SRC}View.cpp
    ${VIS_SRC}VaporTrail.cpp
    ${VIS_SRC}VaporTrailStorage.cpp
    ${VIS_SRC}VelocityVector.cpp
    ${VIS_SRC}Viewer.cpp
    ${VIS_SRC}ViewManager.cpp
    ${VIS_SRC}ViewManagerLogDbAdapter.cpp
)

set(VIS_SOURCES_DB
    ${VIS_INC}DBFormat.cpp
    ${VIS_INC}DB/QSError.cpp
    ${VIS_INC}DB/QSNodeID96.cpp
    ${VIS_INC}DB/QSPosXYExtents.cpp
    ${VIS_INC}DB/SQLiteDataBaseReadUtil.cpp
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
    ${VIS_SRC}RFProp/PPFDataProvider.cpp
    ${VIS_SRC}RFProp/Profile.cpp
    ${VIS_SRC}RFProp/ProfileContext.cpp
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
    ${VIS_SRC}GOG/Cone.cpp
    ${VIS_SRC}GOG/Cylinder.cpp
    ${VIS_SRC}GOG/Ellipse.cpp
    ${VIS_SRC}GOG/Ellipsoid.cpp
    ${VIS_SRC}GOG/GOGNode.cpp
    ${VIS_SRC}GOG/GogNodeInterface.cpp
    ${VIS_SRC}GOG/Hemisphere.cpp
    ${VIS_SRC}GOG/ImageOverlay.cpp
    ${VIS_SRC}GOG/LatLonAltBox.cpp
    ${VIS_SRC}GOG/Line.cpp
    ${VIS_SRC}GOG/LineSegs.cpp
    ${VIS_SRC}GOG/Loader.cpp
    ${VIS_SRC}GOG/LoaderUtils.cpp
    ${VIS_SRC}GOG/Orbit.cpp
    ${VIS_SRC}GOG/ParsedShape.cpp
    ${VIS_SRC}GOG/Points.cpp
    ${VIS_SRC}GOG/Polygon.cpp
    ${VIS_SRC}GOG/Sphere.cpp
    ${VIS_SRC}GOG/Utils.cpp
)

# simVis::GOG::Parser, deprecated code, depends on other deprecated code
if(MSVC)
    set_source_files_properties(${VIS_SRC}GOG/Parser.cpp PROPERTIES COMPILE_FLAGS -wd4996)
else()
    set_source_files_properties(${VIS_SRC}GOG/Parser.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

# Detect EarthManipulator's bindTouchDrag() needed for better touch support
include(CheckOSGEarthMethodExists)
check_osgearth_method_exists(HAVE_EARTHMANIPULATOR_BINDTOUCHDRAG "osgEarth/EarthManipulator" "bindTouchDrag")

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
source_group("Source Files" FILES ${VIS_SOURCES_CORE})

source_group(Headers\\RFProp FILES ${VIS_HEADERS_RFPROP})
source_group("Source Files\\RFProp" FILES ${VIS_SOURCES_RFPROP})

source_group(Headers\\GOG FILES ${VIS_HEADERS_GOG})
source_group("Source Files\\GOG" FILES ${VIS_SOURCES_GOG})

set(VIS_HEADERS ${VIS_HEADERS_CORE} ${VIS_HEADERS_RFPROP} ${VIS_HEADERS_GOG})
set(VIS_SOURCES ${VIS_SOURCES_CORE} ${VIS_SOURCES_RFPROP} ${VIS_SOURCES_GOG} ${VIS_SHADER_FILES})

# Add DB Format support to right MSVC folders and headers for installation
if(TARGET SQLite::SQLite3)
    source_group(Headers FILES ${VIS_HEADERS_DB})
    source_group(Headers\\DB FILES ${VIS_HEADERS_DB_PRIVATE})
    source_group("Source Files\\DB" FILES ${VIS_SOURCES_DB})

    list(APPEND VIS_HEADERS_CORE ${VIS_HEADERS_DB})
    list(APPEND VIS_HEADERS ${VIS_HEADERS_DB} ${VIS_HEADERS_DB_PRIVATE})
    list(APPEND VIS_SOURCES ${VIS_SOURCES_DB})
endif()

# ----------------------------------------------------------------------

# Avoid false MSVC 2017/2019 MSB8027 warning from Unity build on Utils.cpp and Angle.cpp
set_source_files_properties(${VIS_SRC}Utils.cpp ${VIS_SRC}Headless.cpp ${VIS_SRC}GOG/Utils.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

set(VIS_PROJECT_FILES
    ${VIS_HEADERS} ${VIS_SOURCES}
)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMVIS_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(simVis ${STATIC_OR_SHARED} ${VIS_PROJECT_FILES})
set_target_properties(simVis PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "simVis"
)
ApplySDKVersion(simVis)
target_include_directories(simVis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(simVis PUBLIC simData simCore simNotify
    ${OSG_ALL_LIBDEPENDENCIES} osgEarth::osgEarth)
target_link_libraries(simVis PRIVATE OpenGL::GL)

if(TARGET SQLite::SQLite3)
    target_link_libraries(simVis PRIVATE SQLite::SQLite3)
    if(SDK_BIG_ENDIAN)
        target_compile_definitions(simVis PRIVATE SIM_BIG_ENDIAN)
    else()
        target_compile_definitions(simVis PRIVATE SIM_LITTLE_ENDIAN)
    endif()
    set(SIMVIS_HAVE_SQLITE ON)
endif()

if(SIMVIS_SHARED)
    target_compile_definitions(simVis PRIVATE simVis_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(simVis PUBLIC simVis_LIB_EXPORT_STATIC)
endif()
if(HAVE_EARTHMANIPULATOR_BINDTOUCHDRAG)
    target_compile_definitions(simVis PRIVATE HAVE_EARTHMANIPULATOR_BINDTOUCHDRAG)
endif()

# Various bits of code check known non-null to null, as part of macros
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wno-nonnull-compare" HAVE_NONNULL_COMPARE)
if(HAVE_NONNULL_COMPARE)
    target_compile_options(simVis PRIVATE "-Wno-nonnull-compare")
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    configure_file(simVisConfig.cmake.in simVisConfig.cmake @ONLY)
    vsi_install_export(simVis ${SIMDIS_SDK_VERSION_STRING} AnyNewerVersion)
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
    PostBuildInstallSharedObjects(simVis)
endif()

# Setup header file installation
if(INSTALL_HEADERS)
    install(FILES ${VIS_HEADERS_CORE}
        DESTINATION include/simVis)
    install(FILES ${VIS_HEADERS_RFPROP}
        DESTINATION include/simVis/RFProp)
    install(FILES ${VIS_HEADERS_GOG}
        DESTINATION include/simVis/GOG)
endif()
