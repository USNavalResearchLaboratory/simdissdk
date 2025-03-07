if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_SIMPLESERVER)

set(PROJECT_FILES
    DataEngine.cpp
    DataEngine.h
    DataGenerators.cpp
    DataGenerators.h
    InstallOcean.cpp
    InstallOcean.h
    ViewerApp.cpp
    ViewerApp.h
    ExampleSimpleServer.cpp
)

add_executable(example_simpleserver ${PROJECT_FILES})
target_link_libraries(example_simpleserver PRIVATE simVis simUtil)
set_target_properties(example_simpleserver PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Simple Server"
)
if(TARGET ImGuiLib)
    target_link_libraries(example_simpleserver PRIVATE ImGuiLib)
    target_compile_definitions(example_simpleserver PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_simpleserver SDK_Examples)

# Link to Triton if available
if(TARGET osgEarth::osgEarthTriton)
    target_link_libraries(example_simpleserver PRIVATE osgEarth::osgEarthTriton)
    target_compile_definitions(example_simpleserver PRIVATE HAVE_OSGEARTH_TRITON)
endif()
