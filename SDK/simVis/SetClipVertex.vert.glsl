#version 140

#pragma vp_entryPoint sim_clip_vert
#pragma vp_location vertex_view

// Assigns gl_ClipVertex to enable FFP clipping operations
void sim_clip_vert(inout vec4 view)
{
  gl_ClipVertex = view;
}
