#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint simutil_terraintoggle_vert
#pragma vp_location   vertex_model
#pragma vp_order      4.0

in vec4 oe_terrain_attr;
uniform bool simutil_terraintoggle_enabled;

// Move the elevation vertex from height back to earth surface to flatten terrain
void simutil_terraintoggle_vert(inout vec4 VertexMODEL)
{
  if (simutil_terraintoggle_enabled)
  {
    vec3  upVector  = oe_terrain_attr.xyz;
    float elev      = oe_terrain_attr.w;
    vec3  offset    = upVector * -elev;
    VertexMODEL    += vec4(offset/VertexMODEL.w, 0.0);
  }
}
