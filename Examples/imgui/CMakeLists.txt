if(NOT TARGET simVis)
    return()
endif()

find_package(GLEW)
if(NOT TARGET GLEW::GLEW OR NOT TARGET osgEarth::osgEarth OR NOT TARGET OpenGL::GL)
    return()
endif()

# g++ 4.8.x does not work with osgEarth ImGUI due to unordered_map issues in C++1y
if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    message(WARNING "Unable to build IMGUI example code; upgrade g++ past 4.8.")
    return()
endif()

# Add the local ImGui to search path. Due to osgEarth incompatibilities with newer
# versions, set this to the front of CMAKE_PREFIX_PATH for VSI compatibility.
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/1.90.2;${CMAKE_PREFIX_PATH}")
# Prepend the user's environment variable, to prefer it first
if(DEFINED ENV{IMGUI_DIR})
    set(CMAKE_PREFIX_PATH "$ENV{IMGUI_DIR};${CMAKE_PREFIX_PATH}")
endif()

# Let user override with either CMAKE_PREFIX_PATH, or IMGUI_DIR environment variable
find_path(ImGui_DIR NAMES imgui.h)

# Must have ImGui_DIR defined to move past this point
if(NOT ImGui_DIR)
    return()
endif()

add_library(imgui_gl3 STATIC
    ${ImGui_DIR}/imgui.h
    ${ImGui_DIR}/imgui.cpp
    ${ImGui_DIR}/imgui_demo.cpp
    ${ImGui_DIR}/imgui_draw.cpp
    ${ImGui_DIR}/imgui_widgets.cpp
    ${ImGui_DIR}/imgui_tables.cpp
    ${ImGui_DIR}/backends/imgui_impl_opengl3.h
    ${ImGui_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_gl3 PUBLIC "${ImGui_DIR}")
target_link_libraries(imgui_gl3 PUBLIC GLEW::GLEW OpenGL::GL)
add_library(VSI::imgui_gl3 ALIAS imgui_gl3)

# Set up the ImGuiLib, based on ImGui
set(SOURCE_FILES
    BaseGui.h
    BaseGui.cpp
    OsgImGuiHandler.h
    OsgImGuiHandler.cpp
    SimExamplesGui.h
    SimExamplesGui.cpp
)

add_library(ImGuiLib ${SOURCE_FILES})
target_include_directories(ImGuiLib PUBLIC .)
target_link_libraries(ImGuiLib PUBLIC osgEarth::osgEarth VSI::imgui_gl3 PRIVATE simVis simCore simNotify)
target_compile_definitions(ImGuiLib PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ImGuiLib PRIVATE -fno-strict-aliasing)
endif()
set_target_properties(ImGuiLib PROPERTIES
    FOLDER "ImGui Library"
    PROJECT_LABEL "ImGui"
)

find_package(Threads)
target_link_libraries(ImGuiLib PRIVATE Threads::Threads)
