#version 140

uniform float threshold;
uniform vec4 belowColor;
uniform vec4 aboveColor;
uniform int mode;

// mode corresponds to ColorProvider::ColorMode

// Returns an above/below threshold value for loss
vec4 lossToColor(in float loss)
{
  // Special case, if the loss value is invalid/no-data
  if (loss < -32765.0)
    return vec4(0.0, 0.0, 0.0, 0.0);
  if (mode == 2) // above and below
  {
    if (loss <= threshold) return belowColor;
    return aboveColor;
  }
  else if (mode == 1 && loss > threshold) return aboveColor; // above only
  else if (mode == 0 && loss <= threshold) return belowColor; // below only
  return vec4(0.0, 0.0, 0.0, 0.0);
}
