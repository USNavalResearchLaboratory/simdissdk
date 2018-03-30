#version 140

#pragma vp_entryPoint simvis_flashing_fragment
#pragma vp_location fragment_coloring

uniform float osg_FrameTime;
uniform bool simvis_flashing_enable;

// Returns alpha to use in order to flash every period seconds (1/hertz)
float simvis_flashAlpha(float period)
{
  // Over a period of [period] seconds, linear change between [0,halfPeriod]
  float halfPeriod = period * 0.5;
  float quarterPeriod = halfPeriod * 0.5;
  // Take linearly increasing value (mod()) and have the second "half" angle downward;
  // upDown will be from [0,halfPeriod] and graphs like ramp up and ramp down over the time period.
  float upDown = abs(halfPeriod - mod(osg_FrameTime, period));

  // Turn off on lower period, and on in upper period.  Use a smooth step between transitions.
  const float IN_OUT_DURATION = 0.05; // seconds
  return smoothstep(quarterPeriod - IN_OUT_DURATION, quarterPeriod + IN_OUT_DURATION, upDown);
}

void simvis_flashing_fragment(inout vec4 color)
{
  if (simvis_flashing_enable)
    color.a *= simvis_flashAlpha(2.0);
}
