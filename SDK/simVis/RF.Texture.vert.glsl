#version 140

#pragma vp_location vertex_view
#pragma vp_entryPoint rf_vertex

uniform float alpha;
out vec4 texcoord;

// Vertex shader for the 3D texture approach of RF prop visualization just assigns texture coordinate
void rf_vertex(inout vec4 VertexVIEW)
{
  texcoord = gl_MultiTexCoord0;
}
