if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ASI_VIEWER)

set(PROJECT_FILES
    ASIViewer.cpp
)

add_executable(example_asiviewer ${PROJECT_FILES})
target_link_libraries(example_asiviewer PRIVATE simVis simUtil)
set_target_properties(example_asiviewer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "ASI Simple Viewer"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_asiviewer PRIVATE ImGuiLib)
    target_compile_definitions(example_asiviewer PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_asiviewer SDK_Examples)
