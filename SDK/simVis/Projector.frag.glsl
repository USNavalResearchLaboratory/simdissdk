#version 330

#pragma vp_entryPoint sim_proj_frag
#pragma vp_location fragment_coloring

#pragma import_defines(SIMVIS_USE_REX)

uniform bool projectorActive;
uniform float projectorAlpha;
uniform sampler2D simProjSampler;
in vec4 simProjTexCoord;
in vec3 simProjToVert_VIEW;
flat in vec3 simProjLookVector_VIEW;
vec3 vp_Normal;

void sim_proj_frag(inout vec4 color)
{
#ifdef SIMVIS_USE_REX
  vec4 outColor = vec4(0.0);
#else
  vec4 outColor = vec4(color.rgb, 0.0);
#endif

  if (projectorActive)
  {
    vec2 local = simProjTexCoord.st / simProjTexCoord.q;               // same as textureProj, but do it manually so we can check extents

    if (clamp(local, 0.0, 1.0) == local)                             // clip to projected texture domain
    {
      if (dot(simProjLookVector_VIEW, simProjToVert_VIEW) >= 0.0)  // only draw in front of projector (not behind)
      {
        float vis = -dot(normalize(simProjToVert_VIEW), normalize(vp_Normal));  // >=0 is visible; <0 is over the horizon.
        float fade = clamp(vis >= 0.0 ? 1.0 : (1.0+20.0*vis), 0.0, 1.0);        // gradual fade at horizon instead of abrupt stop
        vec4 textureColor = texture(simProjSampler, local);
        outColor = vec4(textureColor.r, textureColor.g, textureColor.b, textureColor.a * projectorAlpha * fade);
      }
    }
  }
  color = outColor;
}
