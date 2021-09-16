if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_MASSIVE_DATA)

set(PROJECT_FILES
    MassiveData.cpp
)

add_executable(example_massivedata ${PROJECT_FILES})
target_link_libraries(example_massivedata PRIVATE simVis simUtil)
set_target_properties(example_massivedata PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Massive Data"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_massivedata PRIVATE ImGuiLib)
    target_compile_definitions(example_massivedata PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_massivedata SDK_Examples)
