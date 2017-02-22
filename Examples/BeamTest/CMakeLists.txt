if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_BEAM)

set(PROJECT_FILES
    src/BeamTest.cpp
)

add_executable(example_beam ${PROJECT_FILES})
target_link_libraries(example_beam PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_beam PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Beams"
)
vsi_install_target(example_beam SDK_Examples)
