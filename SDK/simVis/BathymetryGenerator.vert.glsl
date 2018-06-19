#version 140

#pragma vp_entryPoint simVis_BathymetryGenerator_vertex
#pragma vp_location vertex_clip

uniform float simVis_BathymetryGenerator_offset;
uniform float simVis_BathymetryGenerator_seaLevel;
float oe_terrain_getElevation();
vec3 oe_UpVectorView; // stage global from osgEarth

void simVis_BathymetryGenerator_vertex(inout vec4 vertex)
{
  float elev = oe_terrain_getElevation();
  if (elev <= simVis_BathymetryGenerator_seaLevel) {
    vertex.xyz += oe_UpVectorView * simVis_BathymetryGenerator_offset;
  }
}
