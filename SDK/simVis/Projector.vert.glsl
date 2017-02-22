#version 330

#pragma vp_entryPoint sim_proj_vert
#pragma vp_location vertex_view

uniform mat4 osg_ViewMatrixInverse;
uniform mat4 simProjTexGenMat;
uniform vec3 simProjDir;
uniform vec3 simProjPos;
uniform mat4 osg_ViewMatrix;
out vec4 simProjTexCoord;
out vec3 simProjToVert_VIEW;
flat out vec3 simProjLookVector_VIEW;  // no interpolation required

void sim_proj_vert(inout vec4 vertex_VIEW)
{
  vec4 vertex_WORLD = osg_ViewMatrixInverse * vertex_VIEW;           // vertex in world coords
  simProjTexCoord = simProjTexGenMat * vertex_WORLD;                 // calculate the texture projection coords
  vec3 simProjToVert_WORLD = vertex_WORLD.xyz - simProjPos;          // vector from projector to vertex (world space)
  simProjToVert_VIEW = mat3(osg_ViewMatrix) * simProjToVert_WORLD;   // ..into view space: mat3 cast for 3x3 vector transform
  simProjLookVector_VIEW = mat3(osg_ViewMatrix) * simProjDir;        // ..into view space: mat3 cast for 3x3 vector transform
}
