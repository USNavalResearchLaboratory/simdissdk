#version 110

#pragma vp_entryPoint simvis_track_vertex
#pragma vp_location vertex_view

uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform bool simvis_track_flatmode;
uniform float simvis_track_flatradius;

void simvis_track_vertex(inout vec4 VertexVIEW)
{
  if (simvis_track_flatmode) {
    vec4 vertexWorld = osg_ViewMatrixInverse * VertexVIEW;
    vec3 vertexWorld3 = vertexWorld.xyz/vertexWorld.w;
    vec3 vertexVector = normalize(vertexWorld3);
    vec4 vertexWorldFlat = vec4(vertexVector * simvis_track_flatradius, 1.0) * vertexWorld.w;
    VertexVIEW = osg_ViewMatrix * vertexWorldFlat; // back to VIEW
  }
}
