# Use the Legacy OpenGL preference instead of GLVND
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL)

if(OPENGL_FOUND)
    add_library(VSI::GL UNKNOWN IMPORTED)
    add_library(VSI::GLU UNKNOWN IMPORTED)

    # Windows returns "opengl32" and "glu32".  Rather than tack .lib on the end, just specify them, possibly more compatible
    if(WIN32)
        set_target_properties(VSI::GL PROPERTIES IMPORTED_LOCATION "opengl32.lib")
        set_target_properties(VSI::GLU PROPERTIES IMPORTED_LOCATION "glu32.lib" INTERFACE_LINK_LIBRARIES "VSI::GL")
    else()
        set_target_properties(VSI::GL PROPERTIES IMPORTED_LOCATION "${OPENGL_gl_LIBRARY}")
        set_target_properties(VSI::GLU PROPERTIES IMPORTED_LOCATION "${OPENGL_glu_LIBRARY}" INTERFACE_LINK_LIBRARIES "VSI::GL")
    endif()
endif()
