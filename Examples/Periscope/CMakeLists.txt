if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_PERISCOPE)

set(PROJECT_FILES
    PeriscopeExample.cpp
)

add_executable(example_periscope ${PROJECT_FILES})
target_link_libraries(example_periscope PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_periscope PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Periscope"
)
vsi_install_target(example_periscope SDK_Examples)
