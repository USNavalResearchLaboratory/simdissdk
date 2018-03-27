#version 330
#pragma vp_entryPoint simvis_mapscale_vert
#pragma vp_location vertex_view

// Use flat output color to avoid gradient
flat out vec4 sv_flat_color;
vec4 vp_Color;
void simvis_mapscale_vert(inout vec4 vertex)
{
  sv_flat_color = vp_Color;
}

[break]

#version 330
#pragma vp_entryPoint simvis_mapscale_frag
#pragma vp_location fragment_coloring

// Assign the color, overriding the incoming gradient color
flat in vec4 sv_flat_color;
void simvis_mapscale_frag(inout vec4 color)
{
  color = sv_flat_color;
}
