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

in vec3 vp_Normal;
in vec3 oe_UpVectorView;
in vec4 simProjTexCoord;
in vec3 simProjToVert_VIEW;
in vec3 simProjLookVector_VIEW;

#ifdef SIMVIS_PROJECT_USE_SHADOWMAP
uniform sampler2D simProjShadowMap;
in vec4 simProjShadowMapCoord;
#endif


// #define PROJ_DEBUG

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
  if (projectorMaxRangeSquared > 0.0 && projectorMaxRangeSquared < dot(simProjToVert_VIEW, simProjToVert_VIEW))
    return;

  float vert_dot_normal = dot(normalize(simProjToVert_VIEW), normalize(vp_Normal));

  vec4 pass_color;
  float fail_mix = 0.0;

#ifdef SIMVIS_PROJECT_USE_SHADOWMAP

  // sample the depth texture as a mask
  const float shadow_bias = 0.00005;
  vec3 shadow_coord = simProjShadowMapCoord.xyz / simProjShadowMapCoord.w;
  float d = textureProj(simProjShadowMap, simProjShadowMapCoord).r; // depth from shadow map
  float z = simProjShadowMapCoord.z / simProjShadowMapCoord.w; // depth of reprojected vertex

  fail_color = vec4(1, 0, 0, 1);
  float a = z + shadow_bias;
  if (d < a) fail_mix = 1.0;
  else if (d >= z) fail_mix = 0.0;
  else fail_mix = 1.0 - ((a - d) / (z - d));

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
