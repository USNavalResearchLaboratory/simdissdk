if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PLATFORM_SYMBOLOGY)

set(PROJECT_FILES
    PlatformSymbology.cpp
)

add_executable(example_platformsymbology ${PROJECT_FILES})
target_link_libraries(example_platformsymbology PRIVATE simVis simUtil)
set_target_properties(example_platformsymbology PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Platform Symbology"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_platformsymbology PRIVATE ImGuiLib)
    target_compile_definitions(example_platformsymbology PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_platformsymbology SDK_Examples)
