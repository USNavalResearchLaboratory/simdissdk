#version $GLSL_VERSION_STR
$OSG_PRECISION_FLOAT

#pragma vp_location vertex_view
#pragma vp_entryPoint rf_vertex

$OSG_VARYING_OUT vec2 texcoord;

// Vertex shader for the 3D texture approach of RF prop visualization just assigns texture coordinate
void rf_vertex(inout vec4 VertexVIEW)
{
  texcoord = gl_MultiTexCoord0.xy;
}