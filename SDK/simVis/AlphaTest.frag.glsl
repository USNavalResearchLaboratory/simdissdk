#version 140

#pragma vp_entryPoint simvis_alpha_test
#pragma vp_location fragment_lighting
#pragma vp_order 300.0

#pragma import_defines(SIMVIS_USE_ALPHA_TEST)

uniform float simvis_alpha_threshold;

void simvis_alpha_test(inout vec4 color)
{
#ifdef SIMVIS_USE_ALPHA_TEST
  if (color.a < simvis_alpha_threshold)
    discard;
#endif
}
