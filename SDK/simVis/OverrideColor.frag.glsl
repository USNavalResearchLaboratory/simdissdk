#version 110

#pragma vp_entryPoint simvis_overridecolor_frag
#pragma vp_location fragment_coloring
#pragma vp_order 2.0

uniform vec4 simvis_overridecolor_color;

void simvis_overridecolor_frag(inout vec4 color)
{
  color *= simvis_overridecolor_color;
}
