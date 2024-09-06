if(NOT TARGET GLEW OR NOT TARGET simVis)
    return()
endif()

# g++ 4.8.x does not work with osgEarth ImGUI due to unordered_map issues in C++1y
if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    message(WARNING "Unable to build IMGUI example code; upgrade g++ past 4.8.")
    return()
endif()

set(SOURCE_FILES
    imgui.h
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_impl_opengl3.h
    imgui_impl_opengl3.cpp
    BaseGui.h
    BaseGui.cpp
    OsgImGuiHandler.h
    OsgImGuiHandler.cpp
    SimExamplesGui.h
    SimExamplesGui.cpp
)

add_library(ImGuiLib ${SOURCE_FILES})
target_include_directories(ImGuiLib PUBLIC . )
target_link_libraries(ImGuiLib PUBLIC VSI::GL GLEW OSGEARTH PRIVATE simVis simCore simNotify)
target_compile_definitions(ImGuiLib PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ImGuiLib PRIVATE -fno-strict-aliasing)
endif()
set_target_properties(ImGuiLib PROPERTIES
    FOLDER "ImGui Library"
    PROJECT_LABEL "ImGui"
)
