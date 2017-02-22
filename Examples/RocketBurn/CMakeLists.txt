if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_ROCKET_BURN)

set(PROJECT_FILES
    RocketBurn.cpp
)

add_executable(example_rocket_burn ${PROJECT_FILES})
target_link_libraries(example_rocket_burn PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_rocket_burn PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Rocket Burn"
)
vsi_install_target(example_rocket_burn SDK_Examples)
