if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_RCS)

set(PROJECT_FILES
    src/ExampleRCS.cpp
)

add_executable(example_rcs ${PROJECT_FILES})
target_link_libraries(example_rcs PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_rcs PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - RCS"
)
vsi_install_target(example_rcs SDK_Examples)
