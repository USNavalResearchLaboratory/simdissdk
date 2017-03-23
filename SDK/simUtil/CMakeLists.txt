if(NOT TARGET SIMDIS_SDKVis OR NOT TARGET SIMDIS_SDKData)
    message(STATUS "Skipping simUtil because of missing dependencies.")
    return()
endif()

# ----- simUtil --------------------------------------------------------

set(SHADER_FILES
    TerrainToggleEffect.vert.glsl
    TerrainToggleEffect.frag.glsl
)

set(UTIL_INC)
set(UTIL_SRC)
set(UTIL_HEADERS
    ${UTIL_INC}DataStoreTestHelper.h
    ${UTIL_INC}DatumConvert.h
    ${UTIL_INC}DbConfigurationFile.h
    ${UTIL_INC}DefaultDataStoreValues.h
    ${UTIL_INC}ExampleControls.h
    ${UTIL_INC}ExampleResources.h
    ${UTIL_INC}GogManager.h
    ${UTIL_INC}HudManager.h
    ${UTIL_INC}LayerFactory.h
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
    ${UTIL_INC}ViewpointMonitor.h
    ${UTIL_INC}ViewpointPositions.h
)
set(UTIL_SOURCES
    ${UTIL_SRC}DataStoreTestHelper.cpp
    ${UTIL_SRC}DatumConvert.cpp
    ${UTIL_SRC}DbConfigurationFile.cpp
    ${UTIL_SRC}DefaultDataStoreValues.cpp
    ${UTIL_SRC}ExampleControls.cpp
    ${UTIL_SRC}ExampleResources.cpp
    ${UTIL_SRC}GogManager.cpp
    ${UTIL_SRC}HudManager.cpp
    ${UTIL_SRC}LayerFactory.cpp
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
    ${UTIL_SRC}ViewpointMonitor.cpp
    ${UTIL_SRC}ViewpointPositions.cpp
)
set(UTIL_GENERATED_HEADERS)
set(UTIL_GENERATED_SOURCES)

set(UTIL_ALL_HEADER
    ${UTIL_INC}../simUtil.h
)

# Append TritonSettings if we have the osgEarth Triton nodekit
if(OSGEARTH_TRITON_SUPPORT STREQUAL "NODEKIT" AND TARGET OSGEARTH_TRITON)
    list(APPEND UTIL_HEADERS ${UTIL_INC}TritonSettings.h)
    list(APPEND UTIL_SOURCES ${UTIL_SRC}TritonSettings.cpp)
endif()

# Append SilverLiningSettings if we have the osgEarth Triton nodekit
if(OSGEARTH_SILVERLINING_SUPPORT STREQUAL "NODEKIT" AND TARGET OSGEARTH_SILVERLINING)
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
source_group(Sources FILES ${UTIL_SOURCES})
source_group("" FILES ${UTIL_ALL_HEADER})

# ----------------------------------------------------------------------

set(UTIL_PROJECT_FILES
    ${UTIL_HEADERS} ${UTIL_SOURCES} ${UTIL_ALL_HEADER} ${UTIL_SHADER_FILES}
)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMUTIL_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(SIMDIS_SDKUtil ${STATIC_OR_SHARED} ${UTIL_PROJECT_FILES})
set_target_properties(SIMDIS_SDKUtil PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "SIMDIS SDK - Util"
)
target_include_directories(SIMDIS_SDKUtil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(SIMDIS_SDKUtil PUBLIC SIMDIS_SDKVis SIMDIS_SDKData SIMDIS_SDKCore)
if(SIMUTIL_SHARED)
    target_compile_definitions(SIMDIS_SDKUtil PRIVATE SIMDIS_SDKUtil_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(SIMDIS_SDKUtil PUBLIC SIMDIS_SDKUtil_LIB_EXPORT_STATIC)
endif()


if(INSTALL_SIMDIS_SDK_LIBRARIES)
    set_target_properties(SIMDIS_SDKUtil PROPERTIES TARGET_EXPORT_NAME "SIMDIS_SDK")
    vsi_install_target(SIMDIS_SDKUtil SDK_Library)
endif()

if(OSGEARTH_TRITON_SUPPORT STREQUAL "NODEKIT" AND TARGET OSGEARTH_TRITON)
    target_link_libraries(SIMDIS_SDKUtil PRIVATE OSGEARTH_TRITON)
endif()
if(OSGEARTH_SILVERLINING_SUPPORT STREQUAL "NODEKIT" AND TARGET OSGEARTH_SILVERLINING)
    target_link_libraries(SIMDIS_SDKUtil PRIVATE OSGEARTH_SILVERLINING)
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
    PostBuildInstallSdkSharedObjects(SIMDIS_SDKUtil)
endif()

if(INSTALL_HEADERS)
    # Setup header file installation
    install(FILES ${UTIL_HEADERS}
        DESTINATION include/simUtil)

    install(FILES ${UTIL_ALL_HEADER}
        DESTINATION include)
endif()
