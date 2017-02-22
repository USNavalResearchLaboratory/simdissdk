if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_PLATFORM_AZIM_ELEV_VIEW)

set(PROJECT_FILES
    src/PlatformAzimElevViewTest.cpp
)

add_executable(example_platform_azim_elev_view ${PROJECT_FILES})
target_link_libraries(example_platform_azim_elev_view PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_platform_azim_elev_view PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Platform AzimElev View"
)
vsi_install_target(example_platform_azim_elev_view SDK_Examples)
