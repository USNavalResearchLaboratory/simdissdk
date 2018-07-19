if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PERISCOPE)

set(PROJECT_FILES
    PeriscopeExample.cpp
)

add_executable(example_periscope ${PROJECT_FILES})
target_link_libraries(example_periscope PRIVATE simVis simUtil)
set_target_properties(example_periscope PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Periscope"
)
vsi_install_target(example_periscope SDK_Examples)

# Pass a variable to compiler if the Triton support is through the NodeKit interface
if(OSGEARTH_TRITON_SUPPORT STREQUAL NODEKIT)
    target_link_libraries(example_periscope PRIVATE OSGEARTH_TRITON)
endif()
