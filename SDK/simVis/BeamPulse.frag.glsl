#version 140

#pragma vp_entryPoint simvis_beampulse_frag
#pragma vp_location fragment_coloring
#pragma vp_order 3.0

// Vertex shader provides the fragment's distance from origin in meters
in float simvis_beampulse_y;

uniform float osg_FrameTime;
uniform bool simvis_beampulse_enabled;
uniform float simvis_beampulse_length; // meters
uniform float simvis_beampulse_rate; // Hz
uniform uint simvis_beampulse_stipplepattern; // bit mask

void simvis_beampulse_frag(inout vec4 color)
{
  if (!simvis_beampulse_enabled || simvis_beampulse_length == 0.0)
    return;

  // Figure out for current time how far along [0,length] we should draw, based on rate (hz) scalar
  float animationDelta = 0.0;
  if (simvis_beampulse_rate != 0.0)
  {
    float interval = 1.0 / simvis_beampulse_rate;
    animationDelta = simvis_beampulse_length * (mod(osg_FrameTime, interval) / interval);
  }
  // Apply the simvis_beampulse_y to the animation so it changes with distance from center
  float valueInLength = mod(animationDelta - simvis_beampulse_y, simvis_beampulse_length);

  // valueInLength is currently in [0,length].  For stippling, we need it to be [0,16] here
  valueInLength = (valueInLength / simvis_beampulse_length) * 16.0;

  // Apply the stippling using bitwise operations
  uint bit = uint(round(valueInLength)) & 15U;
  // If bit matches 0, then the stipple is off
  if ((simvis_beampulse_stipplepattern & (1U << bit)) == 0U)
    color.a = 0.0;
}
