if(NOT TARGET simVis OR NOT TARGET simData)
    message(STATUS "Skipping simUtil because of missing dependencies.")
    return()
endif()

set(SIMUTIL_HAVE_GDAL OFF)
set(SIMUTIL_HAVE_SQLITE3 OFF)

# ----- simUtil --------------------------------------------------------

set(SHADER_FILES
    MapScale.glsl
    TerrainToggleEffect.vert.glsl
    TerrainToggleEffect.frag.glsl
    VelocityParticleLayer.compute.dir.frag.glsl
    VelocityParticleLayer.compute.pos.frag.glsl
    VelocityParticleLayer.compute.vert.glsl
    VelocityParticleLayer.part.frag.glsl
    VelocityParticleLayer.part.vert.glsl
)

set(UTIL_INC)
set(UTIL_SRC)
set(UTIL_HEADERS
    ${UTIL_SRC}DynamicSelectionPicker.h
    ${UTIL_INC}DataStoreTestHelper.h
    ${UTIL_INC}DatumConvert.h
    ${UTIL_INC}DefaultDataStoreValues.h
    ${UTIL_INC}ExampleControls.h
    ${UTIL_INC}ExampleResources.h
    ${UTIL_INC}GestureProcessor.h
    ${UTIL_INC}GogManager.h
    ${UTIL_INC}GridTransform.h
    ${UTIL_INC}HudManager.h
    ${UTIL_INC}HudPositionEditor.h
    ${UTIL_INC}HudPositionManager.h
    ${UTIL_INC}IdMapper.h
    ${UTIL_INC}LayerFactory.h
    ${UTIL_INC}LineGraphic.h
    ${UTIL_INC}MapScale.h
    ${UTIL_INC}MouseDispatcher.h
    ${UTIL_INC}MouseManipulator.h
    ${UTIL_INC}MousePositionManipulator.h
    ${UTIL_INC}NullSkyModel.h
    ${UTIL_INC}PlatformPopupManipulator.h
    ${UTIL_INC}PlatformSimulator.h
    ${UTIL_INC}RecenterEyeOnArea.h
    ${UTIL_INC}Replaceables.h
    ${UTIL_INC}ResizeViewManipulator.h
    ${UTIL_INC}ScreenCoordinateCalculator.h
    ${UTIL_INC}StatsHandler.h
    ${UTIL_INC}StatsSizeFixer.h
    ${UTIL_INC}StatusText.h
    ${UTIL_INC}TerrainToggleEffect.h
    ${UTIL_INC}UnitTypeConverter.h
    ${UTIL_INC}VelocityParticleLayer.h
    ${UTIL_INC}ViewpointMonitor.h
    ${UTIL_INC}ViewpointPositions.h
)
set(UTIL_SOURCES
    ${UTIL_SRC}DynamicSelectionPicker.cpp
    ${UTIL_SRC}DataStoreTestHelper.cpp
    ${UTIL_SRC}DatumConvert.cpp
    ${UTIL_SRC}DefaultDataStoreValues.cpp
    ${UTIL_SRC}ExampleControls.cpp
    ${UTIL_SRC}ExampleResources.cpp
    ${UTIL_SRC}GestureProcessor.cpp
    ${UTIL_SRC}GogManager.cpp
    ${UTIL_SRC}GridTransform.cpp
    ${UTIL_SRC}HudManager.cpp
    ${UTIL_SRC}HudPositionEditor.cpp
    ${UTIL_SRC}HudPositionManager.cpp
    ${UTIL_SRC}IdMapper.cpp
    ${UTIL_SRC}LayerFactory.cpp
    ${UTIL_SRC}LineGraphic.cpp
    ${UTIL_SRC}MapScale.cpp
    ${UTIL_SRC}MouseDispatcher.cpp
    ${UTIL_SRC}MouseManipulator.cpp
    ${UTIL_SRC}MousePositionManipulator.cpp
    ${UTIL_SRC}PlatformPopupManipulator.cpp
    ${UTIL_SRC}PlatformSimulator.cpp
    ${UTIL_SRC}RecenterEyeOnArea.cpp
    ${UTIL_SRC}Replaceables.cpp
    ${UTIL_SRC}ResizeViewManipulator.cpp
    ${UTIL_SRC}ScreenCoordinateCalculator.cpp
    ${UTIL_SRC}StatsHandler.cpp
    ${UTIL_SRC}StatusText.cpp
    ${UTIL_SRC}TerrainToggleEffect.cpp
    ${UTIL_SRC}UnitTypeConverter.cpp
    ${UTIL_SRC}VelocityParticleLayer.cpp
    ${UTIL_SRC}ViewpointMonitor.cpp
    ${UTIL_SRC}ViewpointPositions.cpp
)

if(TARGET SQLite::SQLite3)
    list(APPEND UTIL_HEADERS ${UTIL_INC}DbConfigurationFile.h)
    list(APPEND UTIL_SOURCES ${UTIL_SRC}DbConfigurationFile.cpp)
    set(SIMUTIL_HAVE_SQLITE3 ON)
endif()

if(TARGET GDAL::GDAL AND TARGET OpenGL::GL)
    list(APPEND UTIL_HEADERS ${UTIL_INC}Capabilities.h)
    list(APPEND UTIL_SOURCES ${UTIL_SRC}Capabilities.cpp)
    set(SIMUTIL_HAVE_GDAL ON)
endif()

set(UTIL_GENERATED_HEADERS)
set(UTIL_GENERATED_SOURCES)

# Append TritonSettings if we have the osgEarth Triton nodekit
if(TARGET osgEarth::osgEarthTriton)
    list(APPEND UTIL_HEADERS ${UTIL_INC}TritonSettings.h)
    list(APPEND UTIL_SOURCES ${UTIL_SRC}TritonSettings.cpp)
endif()

# Append SilverLiningSettings if we have the osgEarth SilverLining nodekit
if(TARGET osgEarth::osgEarthSilverLining)
    list(APPEND UTIL_HEADERS ${UTIL_INC}SilverLiningSettings.h)
    list(APPEND UTIL_SOURCES ${UTIL_SRC}SilverLiningSettings.cpp)
endif()

# Note that shaders do not go into simUtil.h and header/cpp don't get installed
set(UTIL_SHADER_FILES
    ${SHADER_FILES}
    ${UTIL_INC}Shaders.h
    ${UTIL_SRC}Shaders.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/Shaders.cpp
)
include(ConfigureShaders)
configure_shaders(Shaders.cpp.in Shaders.cpp ${SHADER_FILES})
source_group(Shaders FILES ${UTIL_SHADER_FILES})

source_group(Headers FILES ${UTIL_HEADERS})
source_group("Source Files" FILES ${UTIL_SOURCES})

# ----------------------------------------------------------------------

set(UTIL_PROJECT_FILES
    ${UTIL_HEADERS} ${UTIL_SOURCES} ${UTIL_SHADER_FILES}
)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMUTIL_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(simUtil ${STATIC_OR_SHARED} ${UTIL_PROJECT_FILES})
set_target_properties(simUtil PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "simUtil"
)
ApplySDKVersion(simUtil)
target_include_directories(simUtil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(simUtil PUBLIC simVis simData simCore)
if(SIMUTIL_SHARED)
    target_compile_definitions(simUtil PRIVATE simUtil_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(simUtil PUBLIC simUtil_LIB_EXPORT_STATIC)
endif()

# Require linking to various osg libraries; these are already required by simVis, so mark required here
target_link_libraries(simUtil PRIVATE ${OPENSCENEGRAPH_LIBRARIES})

if(TARGET osgEarth::osgEarthTriton)
    target_link_libraries(simUtil PRIVATE osgEarth::osgEarthTriton)
endif()
if(TARGET osgEarth::osgEarthSilverLining)
    target_link_libraries(simUtil PRIVATE osgEarth::osgEarthSilverLining)
endif()
if(TARGET SQLite::SQLite3)
    target_compile_definitions(simUtil PRIVATE SIM_HAVE_DB_SUPPORT)
endif()
if(TARGET GDAL::GDAL AND TARGET OpenGL::GL)
    # Required by Capabilities.cpp
    target_link_libraries(simUtil PRIVATE GDAL::GDAL OpenGL::GL)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    configure_file(simUtilConfig.cmake.in simUtilConfig.cmake @ONLY)
    vsi_install_export(simUtil ${SIMDIS_SDK_VERSION_STRING} AnyNewerVersion)
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

if(AUTOINSTALL_LIBS AND SIMUTIL_SHARED)
    PostBuildInstallSharedObjects(simUtil)
endif()

if(INSTALL_HEADERS)
    # Setup header file installation
    install(FILES ${UTIL_HEADERS}
        DESTINATION include/simUtil)
endif()
