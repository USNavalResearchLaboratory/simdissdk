#version 140

#pragma vp_entryPoint simvis_glowhighlight_frag
#pragma vp_location fragment_coloring
#pragma vp_order 3.0

// Overall color of the highlight
uniform vec4 simvis_glowhighlight_color;
// Flag for whether the highlight is enabled or not
uniform bool simvis_glowhighlight_enabled;
// OSG built-in for frame time
uniform float osg_FrameTime;

// Fragment shader that handles the 'glow highlight' capability
void simvis_glowhighlight_frag(inout vec4 color)
{
  // Different behavior based on whether enabled or not
  if (simvis_glowhighlight_enabled)
  {
    float glowPct = sin(osg_FrameTime * 6.0);
    // 0.2 increases the base color from black to white; increase this to make the cycles
    //   seem brighter in contrast to the black on the low end
    // 0.4 increases the amplitude of color range; increase this to make the colors vary
    //   more wildly (both towards white and towards black), or lower it to make the
    //   blink seem more muted, closer to the original color
    color.rgb += 0.2 + 0.4 * glowPct;
    color *= simvis_glowhighlight_color;
  }
}
