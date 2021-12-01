if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_RCS)

set(PROJECT_FILES
    ExampleRCS.cpp
)

add_executable(example_rcs ${PROJECT_FILES})
target_link_libraries(example_rcs PRIVATE simVis simUtil)
set_target_properties(example_rcs PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "RCS"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_rcs PRIVATE ImGuiLib)
    #target_compile_definitions(example_rcs PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_rcs SDK_Examples)
