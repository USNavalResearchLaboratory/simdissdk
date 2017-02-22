#version 130

// Fallback implementation of loss-to-color shader
vec4 lossToColor(in float loss)
{
  return vec4(1.0, 0.0, 1.0, 1.0);
}
