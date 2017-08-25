#version 110

#pragma vp_entryPoint simvis_track_fragment
#pragma vp_location fragment_coloring

uniform vec4 simvis_track_overridecolor;
uniform bool simvis_track_enable;

void simvis_track_fragment(inout vec4 color)
{
  // use the alpha channel to enable/disable override.
  if (simvis_track_enable)
  {
    color.rgb = simvis_track_overridecolor.rgb;
    color.a *= simvis_track_overridecolor.a;
  }
}
