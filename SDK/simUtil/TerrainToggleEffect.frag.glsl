#version $GLSL_VERSION_STR

#pragma vp_entryPoint simutil_terraintoggle_frag
#pragma vp_location   fragment_coloring
#pragma vp_order      4.0

uniform bool oe_isPickCamera;
uniform vec4 oe_terrain_color;
uniform bool simutil_terraintoggle_enabled;

// Move the elevation vertex from height back to earth surface to flatten terrain
void simutil_terraintoggle_frag(inout vec4 color)
{
  if (simutil_terraintoggle_enabled && !oe_isPickCamera)
  {
    color = oe_terrain_color;
  }
}
