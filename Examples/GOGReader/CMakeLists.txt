if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_GOG_READER)

set(PROJECT_FILES
    GOGReader.cpp
)

add_executable(example_gogreader ${PROJECT_FILES})
target_link_libraries(example_gogreader PRIVATE simVis simUtil)
set_target_properties(example_gogreader PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "GOG Reader"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_gogreader PRIVATE ImGuiLib)
    target_compile_definitions(example_gogreader PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_gogreader SDK_Examples)
