#version 140

#pragma vp_entryPoint $ENTRY_POINT
#pragma vp_location fragment_coloring

// this is the method passed to the shader to apply to each pixel
// for a straight 1-channel LUMINANCE image, r == g == b == luminance, a == 1.0
// pixels with  0 < alpha < 0.9 are presumed to be artifacts
// relies on .z to indicate whether the field is enabled or not

// Each shader instance needs its own uniform; this is a float that stores
// x as clear threshold, y  as opaque threshold, and z as an enable field.
uniform vec3 $UNIFORM_NAME;

// Dynamically named entry point for the terrain shader.  Will take the luminance
// value from the color.r component and scale the transparency of the fragment
void $ENTRY_POINT(inout vec4 color)
{
  if ($UNIFORM_NAME.z > 0.5)
  {
    float clear = $UNIFORM_NAME.x;
    float opaque = $UNIFORM_NAME.y;
    float luminance = color.r;
    if (luminance <= clear || color.a < 0.9)
      color.a = 0.0;
    else if (luminance >= opaque)
      color.a = 1.0;
    else
      color.a = (luminance - clear) / (opaque - clear);
  }
}
