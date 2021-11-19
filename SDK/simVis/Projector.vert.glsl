#version $GLSL_VERSION_STR

#pragma vp_entryPoint sim_proj_vert
#pragma vp_location vertex_view

uniform mat4 osg_ViewMatrixInverse;
uniform mat4 simProjTexGenMat;
uniform mat4 simProjShadowMapMat;
uniform vec3 simProjDir;
uniform vec3 simProjPos;
uniform mat4 osg_ViewMatrix;
out vec4 simProjTexCoord;
out vec4 simProjShadowMapCoord;
out vec3 simProjToVert_VIEW;
out vec3 simProjLookVector_VIEW;

void sim_proj_vert(inout vec4 vertex_VIEW)
{
  simProjTexCoord = simProjTexGenMat * vertex_VIEW; // vertex into projected texture space

  simProjShadowMapCoord = simProjShadowMapMat * vertex_VIEW;  // vertex into shadow map texture space

  vec4 vertex_WORLD = osg_ViewMatrixInverse * vertex_VIEW; // vertex in world coords
  vec3 simProjToVert_WORLD = vertex_WORLD.xyz - simProjPos; // vector from projector to vertex (world space)
  mat3 vm3 = mat3(osg_ViewMatrix);
  simProjToVert_VIEW = vm3 * simProjToVert_WORLD;   // ..into view space: mat3 cast for 3x3 vector transform
  simProjLookVector_VIEW = vm3 * simProjDir;        // ..into view space: mat3 cast for 3x3 vector transform
}
