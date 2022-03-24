#version $GLSL_VERSION_STR

#pragma vp_function planetarium_texture_vert vertex_view

// Copy texture coordinates to the fragment shader
out vec2 planet_texcoord;

// Responsible for setting up texture coordinates for rendering on the planetarium
void planetarium_texture_vert(inout vec4 vertex)
{
  planet_texcoord = gl_MultiTexCoord0.st;
}

[break]

#version $GLSL_VERSION_STR
#pragma vp_function planetarium_texture_frag fragment
// Should process early, ahead of projectors, so projectors blend into this instead of other way around
#pragma vp_order 0.5

#pragma import_defines(SIMVIS_PLANETARIUM_NUM_TEXTURES)

// Input texture coordinates from vertex shader
in vec2 planet_texcoord;

// Describes texturing parameters for a planetarium
struct sv_Planetarium_Parameters {
  // Sampler (texture) to render
  sampler2D sampler;
  // Coordinates in order: minimum longitude, maximum longitude, minimum latitude, maximum latitude, degrees
  vec4 coords;
  // Alpha for the mixing, 0.0 to 1.0
  float alpha;
  // True to enable rendering the texture
  bool enabled;
};
// Input values of all planetarium parameters
uniform sv_Planetarium_Parameters sv_planet_tex[SIMVIS_PLANETARIUM_NUM_TEXTURES];

// Given range [edge0, edge1], maps x into a linear space of [0,1]. Output < 0 and > 1 possible if outside edges.
float sv_planet_map0to1(float edge0, float edge1, float x)
{
  return (x - edge0) / (edge1 - edge0);
}

// Fragment shader that renders a texture onto a planetarium sphere
void planetarium_texture_frag(inout vec4 color)
{
  for (int k = 0; k < SIMVIS_PLANETARIUM_NUM_TEXTURES; ++k)
  {
    if (sv_planet_tex[k].enabled)
    {
      // Coverage: .xy is -180 to 180 for X coordinates
      //           .zw is -90 to 90 for Y coordinates
      // Note, unable to use step/smoothstep because we want return values outside range of [0,1]

      float pctX = sv_planet_map0to1(
        (180.0 + sv_planet_tex[k].coords.x) / 360.0,
        (180.0 + sv_planet_tex[k].coords.y) / 360.0,
        planet_texcoord.s);
      float pctY = sv_planet_map0to1(
        (90.0 + sv_planet_tex[k].coords.z) / 180.0,
        (90.0 + sv_planet_tex[k].coords.w) / 180.0,
        planet_texcoord.t);

      // If .st is outside the range, don't alter the color pixel
      if (pctX < 0.0 || pctX > 1.0 || pctY < 0.0 || pctY > 1.0)
        continue;
      vec4 texel = texture(sv_planet_tex[k].sampler, vec2(pctX, pctY));

      // Do RGB color mixing using 1-alpha formula
      float applyAlpha = texel.a * sv_planet_tex[k].alpha;
      color.rgb = color.rgb * (1.0 - applyAlpha) + texel.rgb * applyAlpha;
    }
  }
}
