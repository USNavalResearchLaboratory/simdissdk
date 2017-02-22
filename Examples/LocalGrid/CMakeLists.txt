if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_LOCAL_GRID)

set(PROJECT_FILES
    LocalGrid.cpp
)

add_executable(example_local_grid ${PROJECT_FILES})
target_link_libraries(example_local_grid PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil SIMDIS_SDKData)
set_target_properties(example_local_grid PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Local Grid"
)
vsi_install_target(example_local_grid SDK_Examples)
