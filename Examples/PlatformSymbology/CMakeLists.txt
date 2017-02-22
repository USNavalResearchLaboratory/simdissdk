if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_PLATFORM_SYMBOLOGY)

set(PROJECT_FILES
    src/PlatformSymbology.cpp
)

add_executable(example_platform_symbology ${PROJECT_FILES})
target_link_libraries(example_platform_symbology PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_platform_symbology PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Platform Symbology"
)
vsi_install_target(example_platform_symbology SDK_Examples)
