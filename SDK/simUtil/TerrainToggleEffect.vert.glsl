#pragma vp_entryPoint simutil_terraintoggle_vert
#pragma vp_location   vertex_view
#pragma vp_order      4.0

uniform bool simutil_terraintoggle_enabled;
vec3 oe_UpVectorView; // stage global from osgEarth
float oe_terrain_getElevation();

// Move the elevation vertex from height back to earth surface to flatten terrain
void simutil_terraintoggle_vert(inout vec4 VertexMODEL)
{
  if (simutil_terraintoggle_enabled)
  {
    float elev = oe_terrain_getElevation();
    if (elev != 0.0)
      VertexMODEL.xyz -= oe_UpVectorView * elev;
  }
}
