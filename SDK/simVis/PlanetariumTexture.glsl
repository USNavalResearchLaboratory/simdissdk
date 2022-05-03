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
uniform bool sv_planet_textureonly;

// Given range [edge0, edge1], maps x into a linear space of [0,1]. Output < 0 and > 1 possible if outside edges.
float sv_planet_maplat0to1(float edge0, float edge1, float x)
{
  return (x - edge0) / (edge1 - edge0);
}

// Given range [edge0, edge1], maps x into a linear space of [0,1].
// Handles longitude wrapping around "dateline". See AngleTest unit test in simCore.
float sv_planet_maplon0to1(float edge0, float edge1, float x)
{
  // Normalize the edge values
  if ((edge0 > 1.0 && edge1 > 1.0) || (edge0 < 0.0 && edge1 < 0.0))
  {
    float delta = floor(edge0);
    edge1 -= delta;
    edge0 -= delta;
  }

  // Still needed for some edge cases, e.g. (1.16, 0.9523, 0.15)
  while ((edge1 > 1.0 && x < edge0) || (edge0 > 1.0 && x < edge1))
    x += 1;
  while ((edge1 < 0.0 && x > edge0) || (edge0 < 0.0 && x > edge1))
    x -= 1;

  // Do a remapping
  float rv0to1 = (x - edge0) / (edge1 - edge0);
  // Can't use fmod because it excludes 1.0; need to return a value from 0.0 to 1.0 inclusive
  while (rv0to1 > 1)
    rv0to1 -= 1;
  while (rv0to1 < 0)
    rv0to1 += 1;
  return rv0to1;
}

// Matches simCore::isAngleBetween
bool isAngleBetween(float testAngle, float fromAngle, float sweep)
{
  // Always work with positive sweep internally
  if (sweep < 0.)
  {
    fromAngle = fromAngle + sweep;
    sweep = -sweep;
  }
  // Perform equivalent of angFix360 on testAngle - fromAngle
  float diff = mod(testAngle - fromAngle, 360.);
  if (diff < 0.)
    diff += 360.;
  return diff <= sweep;
}

// Fragment shader that renders a texture onto a planetarium sphere
void planetarium_texture_frag(inout vec4 color)
{
  bool haveTexel = false;
  bool haveValidTexture = false;

  for (int k = 0; k < SIMVIS_PLANETARIUM_NUM_TEXTURES; ++k)
  {
    if (sv_planet_tex[k].enabled)
    {
      haveValidTexture = true;

      // Coverage: .xy is -360 to 360 for X coordinates
      //           .zw is -90 to 90 for Y coordinates

      // Expecting range of [-360, 360] for imageLon1, imageLon2
      float imageLon1 = sv_planet_tex[k].coords.x;
      float imageLon2 = sv_planet_tex[k].coords.y;
      // Expecting a range of [-180, 180] for sphereLon
      float sphereLon = planet_texcoord.s * 360.0 - 180.0;

      // Bound the wrapped longitude angles
      if (!isAngleBetween(sphereLon, imageLon1, imageLon2 - imageLon1))
        continue;

      // Note, unable to use step/smoothstep because we want return values outside range of [0,1]
      float pctX = sv_planet_maplon0to1(
        (180.0 + imageLon1) / 360.0,
        (180.0 + imageLon2) / 360.0,
        planet_texcoord.s);

      float pctY = sv_planet_maplat0to1(
        (90.0 + sv_planet_tex[k].coords.z) / 180.0,
        (90.0 + sv_planet_tex[k].coords.w) / 180.0,
        planet_texcoord.t);

      // If latitude percentage is outside the range, don't alter the color pixel
      if (pctY < 0.0 || pctY > 1.0)
        continue;
      vec4 texel = texture(sv_planet_tex[k].sampler, vec2(pctX, pctY));
      haveTexel = true;

      // Do RGB color mixing using 1-alpha formula
      float applyAlpha = texel.a * sv_planet_tex[k].alpha;
      color.rgb = color.rgb * (1.0 - applyAlpha) + texel.rgb * applyAlpha;
    }
  }

  // Process "texture only" mode, disabling the fragment if no texel applies
  if (sv_planet_textureonly && haveValidTexture && !haveTexel)
    discard;
}
