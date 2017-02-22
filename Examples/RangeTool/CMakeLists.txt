if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_RANGE_TOOL)

set(PROJECT_FILES
    src/ExampleRangeTool.cpp
)

add_executable(example_range_tool ${PROJECT_FILES})
target_link_libraries(example_range_tool PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_range_tool PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Range Tool"
)
vsi_install_target(example_range_tool SDK_Examples)
