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
    PROJECT_LABEL "Periscope"
)
vsi_install_target(example_periscope SDK_Examples)

# Link to Triton and SilverLining if available
if(HAVE_OSGEARTH_TRITON)
    target_link_libraries(example_periscope PRIVATE OSGEARTH_TRITON)
endif()
if (HAVE_OSGEARTH_SILVERLINING)
    target_link_libraries(example_periscope PRIVATE OSGEARTH_SILVERLINING)
endif()

