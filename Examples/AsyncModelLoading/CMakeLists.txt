if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ASYNC_MODEL_LOADING)

set(PROJECT_FILES
    main.cpp
)

add_executable(example_asyncmodelloading ${PROJECT_FILES})
target_link_libraries(example_asyncmodelloading PRIVATE simVis simUtil)
set_target_properties(example_asyncmodelloading PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Asynchronous Model Loading"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_asyncmodelloading PRIVATE ImGuiLib)
    target_compile_definitions(example_asyncmodelloading PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_asyncmodelloading SDK_Examples)
