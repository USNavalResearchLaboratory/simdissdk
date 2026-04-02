#version 330 core
#pragma vp_entryPoint simvis_heatCaptureLocalPos
#pragma vp_location vertex_model

out vec3 svheat_ModelPosition;

// Capture the model coordinates for use in fragment shader
void simvis_heatCaptureLocalPos(inout vec4 vertexModel)
{
  svheat_ModelPosition = vertexModel.xyz;
}

[break]

#version 330 core
#pragma vp_entryPoint simvis_applyPointHeatMap
#pragma vp_location fragment_lighting
#pragma vp_order 4.0

in vec3 svheat_ModelPosition;

uniform int svheat_NumSources;
uniform vec3 svheat_Positions[4]; // Array size must match MAX_HEAT_POINTS in HeatMapSystem.h
uniform vec3 svheat_Params[4];  // x = radius, y = intensity, z = falloff
uniform sampler1D svheat_Gradient; // 256px 1D Lookup Table

void simvis_applyPointHeatMap(inout vec4 color) {
  // Prevent out-of-bounds loops
  int numSources = clamp(svheat_NumSources, 0, 4);
  if (numSources == 0)
    return;

  float totalHeat = 0.0;
  vec3 fragPos = svheat_ModelPosition;

  for(int i = 0; i < numSources; i++) {
    vec3 pos = svheat_Positions[i];
    float radius = svheat_Params[i].x;

    float dist = distance(fragPos, pos);
    if (dist >= radius)
      continue;

    float intensity = svheat_Params[i].y;
    float falloff = svheat_Params[i].z;

    float attenuation = 1.0 - (dist / radius);
    float localizedHeat = pow(attenuation, falloff) * intensity;
    totalHeat += localizedHeat;
  }

  // Only sample the texture and blend if there is actual heat on this pixel
  if (totalHeat > 0.0) {
    totalHeat = clamp(totalHeat, 0.0, 1.0);
    vec4 heatOverlay = texture(svheat_Gradient, totalHeat);
    color.rgb = mix(color.rgb, heatOverlay.rgb, heatOverlay.a);
  }
}
