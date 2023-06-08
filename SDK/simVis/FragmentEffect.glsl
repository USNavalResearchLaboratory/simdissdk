#version $GLSL_VERSION_STR

#pragma vp_entryPoint sim_frageffect_fs
#pragma vp_location fragment_coloring
#pragma vp_order 4.0

// Local copy of simData constants to determine which mode to use
const int SVFE_NONE = 0;
const int SVFE_FORWARD_STRIPE = 1;
const int SVFE_BACKWARD_STRIPE = 2;
const int SVFE_HORIZONTAL_STRIPE = 3;
const int SVFE_VERTICAL_STRIPE = 4;
const int SVFE_CHECKERBOARD = 5;
const int SVFE_DIAMOND = 6;
const int SVFE_GLOW = 7;
const int SVFE_FLASH = 8;

// For stippling effects, changes the hide/show ratio
const float SVFE_TOTAL_PX = 8.0;
const float SVFE_HIDDEN_PX = 2.0;

// For glowing effects, changes the period for glow bright to dim and back again
const float SVFE_GLOW_PERIOD = 6.0;
const vec4 SVFE_GLOW_COLOR = vec4(1.0, 1.0, 1.0, 1.0);

// For flashing effects, changes the amount of time the icon is visible or hidden
const float SVFE_FLASH_VISIBLE = 0.6;
const float SVFE_FLASH_HIDDEN = SVFE_FLASH_VISIBLE;

// Input effect
uniform int svfe_effect;

// OSG built-in for frame time
uniform float osg_FrameTime;

void sim_frageffect_fs(inout vec4 color)
{
  if (svfe_effect == SVFE_NONE) {
    return;
  } else if (svfe_effect == SVFE_FORWARD_STRIPE) {
    if (mod(gl_FragCoord.x * 0.5 - gl_FragCoord.y, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX)
      discard;
  } else if (svfe_effect == SVFE_BACKWARD_STRIPE) {
    if (mod(gl_FragCoord.x * 0.5 + gl_FragCoord.y, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX)
      discard;
  } else if (svfe_effect == SVFE_HORIZONTAL_STRIPE) {
    if (mod(gl_FragCoord.y, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX)
      discard;
  } else if (svfe_effect == SVFE_VERTICAL_STRIPE) {
    if (mod(gl_FragCoord.x, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX)
      discard;
  } else if (svfe_effect == SVFE_CHECKERBOARD) {
    if (mod(gl_FragCoord.x, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX || mod(gl_FragCoord.y, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX)
      discard;
  } else if (svfe_effect == SVFE_DIAMOND) {
    // Combine forward and backward stripe
    if (mod(gl_FragCoord.x * 0.5 - gl_FragCoord.y, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX ||
      mod(gl_FragCoord.x * 0.5 + gl_FragCoord.y, SVFE_TOTAL_PX) < SVFE_HIDDEN_PX)
      discard;
  } else if (svfe_effect == SVFE_GLOW) {
    float glowPct = sin(osg_FrameTime * SVFE_GLOW_PERIOD);
    // 0.2 increases the base color from black to white; increase this to make the cycles
    //   seem brighter in contrast to the black on the low end
    // 0.4 increases the amplitude of color range; increase this to make the colors vary
    //   more wildly (both towards white and towards black), or lower it to make the
    //   blink seem more muted, closer to the original color
    color.rgb += 0.2 + 0.4 * glowPct;
    color *= SVFE_GLOW_COLOR;
  } else if (svfe_effect == SVFE_FLASH) {
    float flashPeriod = mod(osg_FrameTime, SVFE_FLASH_VISIBLE + SVFE_FLASH_HIDDEN);
    if (flashPeriod > SVFE_FLASH_VISIBLE)
      discard;
  }
}
