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
    PROJECT_LABEL "Example - Simple Server"
)
vsi_install_target(example_simpleserver SDK_Examples)

# Newer than 1.10.2 release adds ScreenSpaceLayoutOptions::setTechnique() to osgEarth
check_osgearth_method_exists(HAVE_OSGEARTH_SCREENSPACELAYOUT_TECHNIQUE "osgEarth/ScreenSpaceLayout" "technique")
if(HAVE_OSGEARTH_SCREENSPACELAYOUT_TECHNIQUE)
    target_compile_definitions(example_simpleserver PRIVATE HAVE_OSGEARTH_SCREENSPACELAYOUT_TECHNIQUE)
endif()

# Pass a variable to compiler if the Triton support is through the NodeKit interface
if(OSGEARTH_TRITON_SUPPORT STREQUAL NODEKIT)
    target_link_libraries(example_simpleserver PRIVATE OSGEARTH_TRITON)
endif()
