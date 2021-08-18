if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOCATOR_TEST)

set(PROJECT_FILES
    LocatorTestExample.cpp
)

add_executable(example_locatortest ${PROJECT_FILES})
target_link_libraries(example_locatortest PRIVATE simVis simUtil)
set_target_properties(example_locatortest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Locator Test"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_locatortest PRIVATE ImGuiLib)
    target_compile_definitions(example_locatortest PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_locatortest SDK_Examples)
