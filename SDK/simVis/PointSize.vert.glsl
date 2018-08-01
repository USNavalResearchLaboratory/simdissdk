#version 140

#pragma vp_entryPoint simvis_point_size
#pragma vp_location vertex_model
#pragma vp_order 1.0

// set a vp_order such that it executes before PointDrawable.glsl from osgEarth

uniform float simvis_pointsize;

void simvis_point_size(inout vec4 vertex)
{
  gl_PointSize = simvis_pointsize;
}
