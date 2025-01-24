if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_OCEAN)

set(PROJECT_FILES
    Ocean.cpp
)

add_executable(example_ocean ${PROJECT_FILES})
target_link_libraries(example_ocean PRIVATE simVis simUtil)
set_target_properties(example_ocean PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Ocean"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_ocean PRIVATE ImGuiLib)
    target_compile_definitions(example_ocean PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_ocean SDK_Examples)

# Link to Triton if available
if(TARGET osgEarth::osgEarthTriton)
    target_link_libraries(example_ocean PRIVATE osgEarth::osgEarthTriton)
endif()
# Link to SilverLining if available
if(TARGET osgEarth::osgEarthSilverLining)
    target_link_libraries(example_ocean PRIVATE osgEarth::osgEarthSilverLining)
endif()
