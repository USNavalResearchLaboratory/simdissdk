#version $GLSL_VERSION_STR
#pragma vp_function sim_proj_vert, vertex_view
#pragma import_defines(SIMVIS_PROJECT_USE_SHADOWMAP)

uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ViewMatrixInverse;
uniform mat4 simProjTexGenMat;
uniform vec3 simProjDir;
uniform vec3 simProjPos;

out vec4 simProjTexCoord;
out vec3 simProjToVert_VIEW;
out vec3 simProjLookVector_VIEW;
out vec4 simProjVert_VIEW;

#if SIMVIS_PROJECT_USE_SHADOWMAP
uniform mat4 simProjShadowMapMat;
out vec4 simProjShadowMapCoord;
#endif

void sim_proj_vert(inout vec4 vertex_VIEW)
{
  // vertex projected into texture space
  simProjTexCoord = simProjTexGenMat * vertex_VIEW; 

  mat3 vm3 = mat3(osg_ViewMatrix);

  // vector from projector to vertex (in view space)
  vec4 vertex_WORLD = osg_ViewMatrixInverse * vertex_VIEW; 
  vec3 simProjToVert_WORLD = vertex_WORLD.xyz - simProjPos; 
  simProjToVert_VIEW = vm3 * simProjToVert_WORLD;

  // projector look-vector in view space
  simProjLookVector_VIEW = vm3 * simProjDir;

#if SIMVIS_PROJECT_USE_SHADOWMAP
  // vertex projected into shadow map space
  simProjShadowMapCoord = simProjShadowMapMat * vertex_VIEW;
  simProjVert_VIEW = vertex_VIEW;
#endif
}
