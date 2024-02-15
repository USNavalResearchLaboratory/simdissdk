#version 140

#pragma vp_entryPoint simvis_overridecolor_frag
#pragma vp_location fragment_coloring
#pragma vp_order 2.0

uniform vec4 simvis_overridecolor_color;
// Combine mode: 0 == off; 1 == multiply; 2 == replace; 3 = intensity
uniform int simvis_overridecolor_combinemode;

void simvis_overridecolor_frag(inout vec4 color)
{
  if (simvis_overridecolor_combinemode == 1)
  {
    color *= simvis_overridecolor_color;
  }
  else if (simvis_overridecolor_combinemode == 2)
  {
    color.rgb = simvis_overridecolor_color.rgb;
    color.a *= simvis_overridecolor_color.a;
  }
}

[break]
#version 140

#pragma vp_entryPoint simvis_overridecolor_intensity_frag
#pragma vp_location fragment_lighting

uniform vec4 simvis_overridecolor_color;
// Combine mode: 0 == off; 1 == multiply; 2 == replace; 3 = intensity
uniform int simvis_overridecolor_combinemode;

void simvis_overridecolor_intensity_frag(inout vec4 color)
{
  if (simvis_overridecolor_combinemode == 3)
  {
    float intensity = (color.r + color.g + color.b) / 3.;
    color = vec4(simvis_overridecolor_color.rgb * intensity, simvis_overridecolor_color.a * color.a);
  }
}
