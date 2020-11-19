#version 330

#pragma import_defines(TEXEL_TO_VELXY(t))

uniform sampler2D texturePosition;
uniform sampler2D velocityMap;
uniform vec2 resolution;
uniform vec4 boundingBox;

layout(location=0) out vec4 out_particle;

// Computes the rotation angle (direction) for a fragment based on the position's input velocity
void main()
{
  vec2 uv = gl_FragCoord.xy / resolution.xy;
  vec4 positionInfo = texture2D( texturePosition, uv );
  vec3 position = positionInfo.xyz;
  vec2 posUV = position.xy;
  // Rescale posUV from world/global coordinates, into image coordinates
  vec2 scaledPosUv = vec2((posUV.x - boundingBox.x) / (boundingBox.z - boundingBox.x),
    (posUV.y - boundingBox.y) / (boundingBox.w - boundingBox.y));
  vec4 velocityTexel = texture2D(velocityMap, scaledPosUv);

#ifdef TEXEL_TO_VELXY
  vec2 velocity = TEXEL_TO_VELXY(velocityTexel);
#else
  // Default image format presumes X and Y encoded in R and G, from -25 to +25 m/s
  vec2 velocity = mix(vec2(-25.0, -25.0), vec2(25.0, 25.0), velocityTexel.rg);
#endif

  float rotateAngle = atan(velocity.x, -velocity.y);
  out_particle.r = rotateAngle;
}
