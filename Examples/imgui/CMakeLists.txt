if (NOT TARGET GLEW)
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
)

add_library(ImGuiLib ${SOURCE_FILES})
target_include_directories(ImGuiLib PUBLIC . )
target_link_libraries(ImGuiLib PUBLIC VSI::GL GLEW OSGEARTH PRIVATE simVis)
set_target_properties(ImGuiLib PROPERTIES
    FOLDER "ImGui Library"
    PROJECT_LABEL "ImGui"
)
