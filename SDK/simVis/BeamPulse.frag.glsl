#version 110

// Pragmas tell SIMDIS what method name is, and where to fit it into shader framework.
// You probably don't need to change this.
#pragma vp_entryPoint simvis_beampulse_frag
#pragma vp_location fragment_coloring
#pragma vp_order 3.0

// This is a constantly-incrementing counter tied to the system time.  OSG auto-generates this.
uniform float osg_FrameTime;
// This is from the vertex shader and is fragment's distance from origin in meters.
in float simvis_beampulse_y;

uniform bool simvis_beampulse_animate;
uniform float simvis_beampulse_period;
uniform float simvis_beampulse_rate;



void simvis_beampulse_frag(inout vec4 color)
{
  // Refactor the valueInPeriod to fit between 0.0 and 1.0
  if (simvis_beampulse_period == 0.0)
    return;
	
  valueInPeriod = valueInPeriod / simvis_beampulse_period;

  if (!simvis_beampulse_animate)
    return;

  // Get a reasonable modulus -- mod twice to avoid overflow or precision issues
  float timeAfterMod = simvis_beampulse_period * simvis_beampulse_rate * osg_FrameTime;
  float valueInPeriod = mod(timeAfterMod - simvis_beampulse_y, simvis_beampulse_period);
  
  // At this point, valueInPeriod is [0.0, period] and is constantly increasing.
  // It's been modified by the period and rate to create a faster or slower pulse
  
  // TODO: Fancy graphics here if desired; stipples, attenuation factors, etc.
  // Below is a simple if/elseif/else to create a simple stipple.  There's probably
  // a better way to do it or make it configurable.

  // Provide two different sized "on" sections to show how stippling could work    
  if ((valueInPeriod < 0.25) ||
      (valueInPeriod >= 0.6 && valueInPeriod < 0.8) ||
      (valueInPeriod >= 0.9))
    color.a = 0.0;
}
