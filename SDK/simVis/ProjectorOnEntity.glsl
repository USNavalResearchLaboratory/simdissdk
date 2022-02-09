#version $GLSL_VERSION_STR
#pragma vp_function sim_proj_on_entity_vert vertex_view
#pragma import_defines(SIMVIS_NUM_PROJECTORS)

uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ViewMatrixInverse;

uniform mat4 simProjTexGenMat[4];
uniform vec3 simProjDir[4];
uniform vec3 simProjPos[4];

out vec4 simProjTexCoord[4];
out vec3 simProjToVert_VIEW[4];
out vec3 simProjLookVector_VIEW[4];

void sim_proj_on_entity_vert(inout vec4 vertex_VIEW)
{
  for(int i=0; i< SIMVIS_NUM_PROJECTORS; ++i)
  {
    // vertex projected into texture space
    simProjTexCoord[i] = simProjTexGenMat[i] * vertex_VIEW;

    mat3 vm3 = mat3(osg_ViewMatrix);

    // vector from projector to vertex (in view space)
    vec4 vertex_WORLD = osg_ViewMatrixInverse * vertex_VIEW;
    vec3 simProjToVert_WORLD = vertex_WORLD.xyz - simProjPos[i];
    simProjToVert_VIEW[i] = vm3 * simProjToVert_WORLD;

    // projector look-vector in view space
    simProjLookVector_VIEW[i] = vm3 * simProjDir[i];
  }
}



[break]
#version $GLSL_VERSION_STR
#pragma vp_function sim_proj_on_entity_frag fragment
#pragma import_defines(SIMVIS_NUM_PROJECTORS)

in vec3 vp_Normal;
in vec3 oe_UpVectorView;

uniform bool projectorActive[4];
uniform float projectorAlpha[4];
uniform sampler2D simProjSampler[4];
uniform bool projectorUseColorOverride[4];
uniform vec4 projectorColorOverride[4];
uniform float projectorMaxRangeSquared[4];

in vec4 simProjTexCoord[4];
in vec3 simProjToVert_VIEW[4];
in vec3 simProjLookVector_VIEW[4];

// for a projector texturing a platform or other model:
void sim_proj_on_entity_frag(inout vec4 color)
{
    for (int i = 0; i < SIMVIS_NUM_PROJECTORS; ++i)
    {
        if (projectorActive[i] && simProjTexCoord[i].q > 0)
        {
            // clip to projected texture domain; otherwise the texture will project
            // even when the target is outside the projection frustum
            vec2 local = simProjTexCoord[i].st / simProjTexCoord[i].q;
            if (clamp(local, 0.0, 1.0) == local)
            {
                vec4 textureColor;
                // Circumvent "error: sampler arrays indexed with non-constant expressions are forbidden in GLSL 1.30 and later"
                if (i == 1)
                    textureColor = textureProj(simProjSampler[1], simProjTexCoord[i]);
                if (i == 2)
                    textureColor = textureProj(simProjSampler[2], simProjTexCoord[i]);
                if (i == 3)
                    textureColor = textureProj(simProjSampler[3], simProjTexCoord[i]);
                else
                    textureColor = textureProj(simProjSampler[0], simProjTexCoord[i]);

                if (projectorUseColorOverride[i])
                {
                    color.rgb = textureColor.rgb * projectorColorOverride[i].rgb;
                    color.a = textureColor.a * projectorAlpha[i];
                }
                else
                {
                    color.rgb = mix(color.rgb, textureColor.rgb, textureColor.a*projectorAlpha[i]);
                }
            }
        }
    }
}
