#version 130

#pragma vp_location fragment_coloring
#pragma vp_entryPoint rf_color

uniform sampler2D texture;
uniform float alpha;
in vec4 texcoord;

// Forward declare the loss function that the particular display method uses
vec4 lossToColor(in float loss);

// Fragment shader for the 3D texture approach of RF prop visualization applies loss color to texture
void rf_color(inout vec4 color)
{
  float loss = texture2D(texture, texcoord.st).r;
  vec4 lossColor = lossToColor(loss);
  lossColor.a *= alpha;
  color = lossColor;
}
