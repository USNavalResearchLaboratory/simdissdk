#version 110

#pragma vp_entryPoint simvis_point_size
#pragma vp_location vertex_model

uniform float simvis_pointsize;

void simvis_point_size(inout vec4 vertex)
{
  gl_PointSize = simvis_pointsize;
}
