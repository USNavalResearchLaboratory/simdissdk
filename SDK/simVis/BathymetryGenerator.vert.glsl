#version $GLSL_VERSION_STR

#pragma vp_entryPoint simVis_BathymetryGenerator_vertex
#pragma vp_location vertex_clip

// TODO: Investigate why vertex_view works in example_ocean, but not in SIMDIS.
// Based on https://github.com/USNavalResearchLaboratory/simdissdk/pull/27 the
// location should be vertex_view to reduce stutter on screen, but this causes
// display issues in SIMDIS.
// #pragma vp_location vertex_view

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
