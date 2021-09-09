#version 140

#pragma vp_entryPoint simvis_track_fragment
#pragma vp_location fragment_coloring

uniform vec4 simvis_track_overridecolor;
uniform bool simvis_track_enable;

void simvis_track_fragment(inout vec4 color)
{
  // use the uniform to enable/disable override.
  if (simvis_track_enable)
  {
    color.rgb = simvis_track_overridecolor.rgb;
    if (color.a > 0.01)
      color.a = simvis_track_overridecolor.a;
    else
      color.a *= simvis_track_overridecolor.a;
  }
}
