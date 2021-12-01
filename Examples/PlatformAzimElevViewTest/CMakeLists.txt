if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PLATFORM_AZIM_ELEV_VIEW)

set(PROJECT_FILES
    PlatformAzimElevViewTest.cpp
)

add_executable(example_platformazimelevviewtest ${PROJECT_FILES})
target_link_libraries(example_platformazimelevviewtest PRIVATE simVis simUtil)
set_target_properties(example_platformazimelevviewtest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Platform AzimElev View Test"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_platformazimelevviewtest PRIVATE ImGuiLib)
    target_compile_definitions(example_platformazimelevviewtest PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_platformazimelevviewtest SDK_Examples)
