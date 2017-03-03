#version 110

#pragma vp_entryPoint simvis_overridecolor_frag
#pragma vp_location fragment_coloring
#pragma vp_order 2.0

uniform vec4 simvis_overridecolor_color;
uniform bool simvis_use_overridecolor;

void simvis_overridecolor_frag(inout vec4 color)
{
  if (simvis_use_overridecolor)
    color *= simvis_overridecolor_color;
}
