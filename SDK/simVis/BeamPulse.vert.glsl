#version 140

#pragma vp_entryPoint simvis_beampulse_vert
#pragma vp_location vertex_model
#pragma vp_order 3.0

out float simvis_beampulse_y;

// Get the y distance of the vertex shader from origin
void simvis_beampulse_vert(inout vec4 vertex)
{
  simvis_beampulse_y = vertex.y / vertex.w;
}
