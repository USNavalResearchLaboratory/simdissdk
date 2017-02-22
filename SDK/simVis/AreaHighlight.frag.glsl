#version 110

#pragma vp_entryPoint simvis_areahighlight_frag
#pragma vp_location fragment_coloring
#pragma vp_order 3.0

uniform float osg_FrameTime;
uniform vec4 simvis_areahighlight_color;

void simvis_areahighlight_frag(inout vec4 color)
{
  color *= simvis_areahighlight_color;
  float pulsedAmplitude = sin(osg_FrameTime * $GLOW_FREQUENCY) * $GLOW_AMPLITUDE;
  color.rgb += pulsedAmplitude;
  color.a *= $GLOW_MINIMUM_ALPHA + pulsedAmplitude;
}
