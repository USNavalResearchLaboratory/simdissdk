#version 130

#pragma vp_location fragment_coloring
#pragma vp_entryPoint rf_color

in vec4 lossColor;

// Fragment shader of the vertex-based approach to RF prop just uses the varying
// value that was calculated in the vertex shader
void rf_color(inout vec4 color)
{
  color = lossColor;
}
