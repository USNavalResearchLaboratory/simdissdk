if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_RANGE_TOOL)

set(PROJECT_FILES
    ExampleRangeTool.cpp
)

add_executable(example_rangetool ${PROJECT_FILES})
target_link_libraries(example_rangetool PRIVATE simVis simUtil)
set_target_properties(example_rangetool PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Range Tool"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_rangetool PRIVATE ImGuiLib)
    target_compile_definitions(example_rangetool PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_rangetool SDK_Examples)
