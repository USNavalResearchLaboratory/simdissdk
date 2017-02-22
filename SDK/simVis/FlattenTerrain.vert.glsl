#version 120

#pragma vp_entryPoint simVis_FlattenTerrainShader_vertex
#pragma vp_location vertex_model
#pragma vp_order 0.5

uniform bool simVis_useFlattenShader;
attribute vec4 oe_terrain_attr;

void simVis_FlattenTerrainShader_vertex(inout vec4 vertex)
{
  if (simVis_useFlattenShader)
  {
    vec3  upVector  = oe_terrain_attr.xyz;
    float elev      = oe_terrain_attr.w;
    vec3  offset    = upVector * -elev;
    vertex    += vec4(offset/vertex.w, 0.0);
  }
}
