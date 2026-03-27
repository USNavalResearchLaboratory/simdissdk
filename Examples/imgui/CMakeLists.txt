if(NOT TARGET simVis OR NOT TARGET osgEarth::osgEarth OR NOT TARGET OpenGL::GL)
    return()
endif()

# g++ 4.8.x does not work with osgEarth ImGui due to unordered_map issues in C++1y
if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    message(WARNING "Unable to build Dear ImGui example code; upgrade g++ past 4.8.")
    return()
endif()

# Try to use find_package() to locate imgui. This will work with newer
# VSI builds and with vcpkg.
find_package(imgui QUIET)
if(imgui_FOUND)
    # Check for imgui::opengl3; if it exists then use that as the library name; includes the GL3 backend
    if(TARGET imgui::imgui_opengl3)
        add_library(VSI::imgui_gl3 ALIAS imgui::imgui_opengl3)
    elseif(TARGET imgui::imgui)
        add_library(VSI::imgui_gl3 ALIAS imgui::imgui)
    else()
        message(WARNING "find_package(imgui) succeeded, but no valid target found.")
        return()
    endif()
endif()

# Fall back to the source code path for Dear ImGui, if the target does not exist
if(NOT TARGET VSI::imgui_gl3)
    # Add the local Dear ImGui to the search path at the end. This way, your own
    # Dear ImGui if it exists ought to be found first.
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_CURRENT_SOURCE_DIR}/1.92")
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
        ${ImGui_DIR}/imgui_tables.cpp
        ${ImGui_DIR}/imgui_widgets.cpp
        ${ImGui_DIR}/backends/imgui_impl_opengl3.h
        ${ImGui_DIR}/backends/imgui_impl_opengl3.cpp
        ${ImGui_DIR}/backends/imgui_impl_opengl3_loader.h
    )
    target_include_directories(imgui_gl3 PUBLIC "${ImGui_DIR}" "${ImGui_DIR}/backends")
    target_link_libraries(imgui_gl3 PUBLIC OpenGL::GL PRIVATE ${CMAKE_DL_LIBS})
    add_library(VSI::imgui_gl3 ALIAS imgui_gl3)
    set_target_properties(imgui_gl3 PROPERTIES
        FOLDER "Dear ImGui"
        PROJECT_LABEL "Dear ImGui"
    )
endif()

# Set up the ImGuiLib, based on ImGui
set(SOURCE_FILES
    OsgImGuiHandler.h
    OsgImGuiHandler.cpp
    SimExamplesGui.h
    SimExamplesGui.cpp
)

add_library(ImGuiLib ${SOURCE_FILES})
target_include_directories(ImGuiLib PUBLIC .)
target_link_libraries(ImGuiLib PUBLIC osgEarth::osgEarth VSI::imgui_gl3 PRIVATE simVis simCore simNotify)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ImGuiLib PRIVATE -fno-strict-aliasing)
endif()
set_target_properties(ImGuiLib PROPERTIES
    FOLDER "Dear ImGui"
    PROJECT_LABEL "Dear ImGui Utilities Library"
)

find_package(Threads)
target_link_libraries(ImGuiLib PRIVATE Threads::Threads)
