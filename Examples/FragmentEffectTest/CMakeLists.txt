if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_FRAGMENT_EFFECT)

set(PROJECT_FILES
    main.cpp
)

add_executable(example_fragmenteffecttest ${PROJECT_FILES})
target_link_libraries(example_fragmenteffecttest PRIVATE simVis simUtil)
set_target_properties(example_fragmenteffecttest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Fragment Effect Test"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_fragmenteffecttest PRIVATE ImGuiLib)
    target_compile_definitions(example_fragmenteffecttest PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_fragmenteffecttest SDK_Examples)
