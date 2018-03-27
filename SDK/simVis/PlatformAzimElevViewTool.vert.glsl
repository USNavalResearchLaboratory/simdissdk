#version 140

#pragma vp_entryPoint simvis_azel_warp_vertex
#pragma vp_location vertex_view

// Warps geometry in the view space's XY plane
void simvis_azel_warp_vertex(inout vec4 VertexVIEW)
{
  vec4 origin = gl_ModelViewMatrix * vec4(0,0,0,1);
  vec4 vector = VertexVIEW - origin;
  float radius = length(vector.xyz);
  float angle  = asin(length(vector.xy) / radius);
  float arclen = radius * angle;
  VertexVIEW.xy = origin.xy + normalize(vector.xy) * arclen;
}
