#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint simutil_vpl_frag
#pragma vp_location   fragment_coloring

in vec4 particle_color;
in float rotate_angle;
uniform sampler2D pointSprite;
uniform bool usePointSprite;
uniform float oe_VisibleLayer_opacityUniform;

// Generates the particle as a sprite or circle
void simutil_vpl_frag(inout vec4 color)
{
  if (usePointSprite)
  {
    float rot = rotate_angle;
    float sinTheta = sin(rot);
    float cosTheta = cos(rot);
    vec2 aboutOrigin = gl_PointCoord - vec2(0.5);
    vec2 rotated = vec2(aboutOrigin.x * cosTheta - aboutOrigin.y * sinTheta,
      aboutOrigin.y * cosTheta + aboutOrigin.x * sinTheta);
    color = texture(pointSprite, rotated + vec2(0.5)) * particle_color;
  }
  else
  {
    color = particle_color;
    // Draw circles
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (length(coord) > 0.5)
      discard;
  }
  color.a *= oe_VisibleLayer_opacityUniform;
}
