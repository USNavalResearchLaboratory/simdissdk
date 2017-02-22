if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_ARTICULATED_MODEL)

set(PROJECT_FILES
    src/ExampleArticulatedModel.cpp
)

add_executable(example_articulatedmodel ${PROJECT_FILES})
target_link_libraries(example_articulatedmodel PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_articulatedmodel PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Articulated Model"
)
vsi_install_target(example_articulatedmodel SDK_Examples)
