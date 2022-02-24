#version $GLSL_VERSION_STR
#pragma vp_function sim_proj_frag, fragment
#pragma import_defines(SIMVIS_USE_REX)
#pragma import_defines(SIMVIS_PROJECT_USE_SHADOWMAP)

uniform bool projectorActive;
uniform float projectorAlpha;
uniform sampler2D simProjSampler;
uniform bool projectorUseColorOverride;
uniform vec4 projectorColorOverride;
uniform float projectorMaxRangeSquared;
uniform bool projectorDoubleSided;
uniform mat4 simProjShadowMapMat;

in vec3 vp_Normal;
in vec3 oe_UpVectorView;
in vec4 simProjTexCoord;
in vec3 simProjToVert_VIEW;
in vec3 simProjLookVector_VIEW;
in vec4 simProjVert_VIEW;

#if SIMVIS_PROJECT_USE_SHADOWMAP
uniform sampler2D simProjShadowMap;
in vec4 simProjShadowMapCoord;
#endif


//#define PROJ_DEBUG

// for a projector texturing the terrain:
void sim_proj_frag(inout vec4 color)
{
#ifdef SIMVIS_USE_REX
  vec4 fail_color = vec4(0.0);
#else
  vec4 fail_color = vec4(color.rgb, 0.0);
#endif

  // default:
  color = fail_color;

  if (!projectorActive)
    return;

  // clip to projected texture domain to the projection frustum
  vec2 local = simProjTexCoord.xy / simProjTexCoord.w;
  if (clamp(local, 0.0, 1.0) != local)
    return;

  // only draw in front of projector (not behind)
  if (dot(simProjLookVector_VIEW, simProjToVert_VIEW) < 0.0)
    return;

  // only draw up to maximum range
  float distanceToVertSq = dot(simProjToVert_VIEW, simProjToVert_VIEW);
  if (projectorMaxRangeSquared > 0.0 && projectorMaxRangeSquared < distanceToVertSq)
    return;

  // cos of the angle at which the projector is viewing a surface.
  // <0 = front face, >0 = back face
  float vert_dot_normal = dot(normalize(simProjToVert_VIEW), normalize(vp_Normal));

  vec4 pass_color;
  fail_color = vec4(1, 0, 0, 1);
  float fail_mix = 0.0;

#if SIMVIS_PROJECT_USE_SHADOWMAP

  // sample the depth texture as a mask
  float shadow_bias = clamp(0.00005 * tan(acos(-vert_dot_normal)), 0.0, 0.00005);
  float shadow_d = textureProj(simProjShadowMap, simProjShadowMapCoord).z; // depth from shadow map
  float vertex_d = simProjShadowMapCoord.z / simProjShadowMapCoord.w; // depth of reprojected vertex
  float vertex_biased_d = vertex_d - shadow_bias; // apply a little bias
  fail_mix = 1.0 - clamp((shadow_d - vertex_biased_d) / shadow_bias, 0.0, 1.0);

#endif // SIMVIS_PROJECT_USE_SHADOWMAP

  float vis;

  // don't draw on any back-facing surface
  if (!projectorDoubleSided)
  {
    vis = -vert_dot_normal;
    if (vis <= 0.0 && fail_mix < 1.0) {
      fail_mix = 1.0;
      fail_color = vec4(1, 1, 0, 1);
    }
  }

  // don't draw on geometry whose plane is very close to the projection vector
  // since it causes nasty artifacts
  vis = abs(vert_dot_normal);
  if (vis < 0.1 && fail_mix < 1.0) {
    fail_mix = 1.0;
    fail_color = vec4(1, 0, 1, 1);
  }

#ifdef PROJ_DEBUG

  pass_color = vec4(0, 1, 0, 1);

#else // !PROJ_DEBUG

  // sample the projector texture:
  vec4 textureColor = texture(simProjSampler, local);
  pass_color = vec4(textureColor.r, textureColor.g, textureColor.b, textureColor.a * projectorAlpha);
  if (projectorUseColorOverride)
  {
      pass_color.rgb = textureColor.rgb * projectorColorOverride.rgb;
      pass_color.a = textureColor.a * projectorAlpha;
  }
  else
  {
      pass_color.rgb = mix(color.rgb, textureColor.rgb, textureColor.a * projectorAlpha);
  }

  fail_color = vec4(0);

#endif // !PROJ_DEBUG

  color = mix(pass_color, fail_color, fail_mix);
}
