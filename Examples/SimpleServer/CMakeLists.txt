if(NOT TARGET SIMDIS_SDKUtil)
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
target_link_libraries(example_simpleserver PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_simpleserver PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Simple Server"
)
vsi_install_target(example_simpleserver SDK_Examples)
