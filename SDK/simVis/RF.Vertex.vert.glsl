#version 140

#pragma vp_location vertex_view
#pragma vp_entryPoint rf_vertex

uniform float alpha;
in float loss;
out vec4 lossColor;

// Forward declare the loss function that the particular display method uses
vec4 lossToColor(in float loss);

// Apply a varying value of loss color based on the loss value attribute
void rf_vertex(inout vec4 VertexVIEW)
{
  vec4 color = lossToColor(loss);
  color.a *= alpha;
  lossColor = color;
}
