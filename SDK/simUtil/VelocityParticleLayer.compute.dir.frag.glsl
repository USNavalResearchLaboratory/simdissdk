#version 330

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
  vec2 scaledVelocity = texture2D(velocityMap, scaledPosUv).rg;
  float rotateAngle = atan(scaledVelocity.x - 0.5, -(scaledVelocity.y - 0.5));
  out_particle.r = rotateAngle;
}
