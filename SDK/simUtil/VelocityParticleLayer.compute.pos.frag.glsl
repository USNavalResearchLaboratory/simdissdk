#version 330

#pragma import_defines(TEXEL_TO_VELXY(t))

uniform sampler2D texturePosition;
uniform sampler2D velocityMap;
uniform vec2 resolution;
uniform float dieSpeed;
uniform float speedFactor;
uniform float dropChance;
uniform vec4 boundingBox;
uniform float osg_DeltaFrameTime;

layout(location=0) out vec4 out_particle;

// Generate a pseudo-random value in the specified range:
float simutil_random(float minValue, float maxValue, vec2 co)
{
  float t = fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
  return minValue + t*(maxValue-minValue);
}

// Calculates new positions, speed, and life for each particle in the texture based on the velocity look-up
void main()
{
  vec2 uv = gl_FragCoord.xy / resolution.xy;

  vec4 positionInfo = texture2D( texturePosition, uv );

  vec3 position = positionInfo.xyz;
  float life = positionInfo.w;

  if (dieSpeed > 0)
    life -= (osg_DeltaFrameTime / dieSpeed);
  vec2 seed = (position.xy + uv);
  float drop = simutil_random(0.0, 1.0, seed);
  // Do not let particles outside of the box
  if (position.x < boundingBox.x || position.x > boundingBox.z || position.y < boundingBox.y || position.y > boundingBox.w)
    life = -1.0;

  // Reset particle
  if (life < 0.0 || dropChance > drop)
  {
    life = simutil_random(0.3, 1.0, seed + 3.4);
    float x = simutil_random(boundingBox.x, boundingBox.z, seed + 1.3);
    float y = simutil_random(boundingBox.y, boundingBox.w, seed + 2.1);
    float z = 0.0;
    position = vec3(x, y, z);
  }

  vec2 posUV = position.xy;
  float uMin = -25.0;
  float uMax = 25.0;
  float vMin = -25.0;
  float vMax = 25.0;

  // Here, posUV is in global coordinates, with [0,0] meaning lat/lon [-180,-90] and [1,1] meaning [180,90].
  // If the life is greater than 0, then we know we have a point inside our bounds.  We want to reproject
  // the values from the posUV space into the image space, which is in boundingBox coordinates.
  // For example, using bounding box X coordinates [min=0.6, max=0.8], and an input posUV value of 0.7,
  // we can reproject by doing (0.7 - 0.6) / (0.8 - 0.6).  This gets a uv in the space of the input image.
  vec4 velocityTexel = vec4(0.0, 0.0, 0.0, 0.0);
  if (life >= 0.0) // Must be inside the bounds if this passes
  {
    vec2 scaledPosUv = vec2((posUV.x - boundingBox.x) / (boundingBox.z - boundingBox.x),
      (posUV.y - boundingBox.y) / (boundingBox.w - boundingBox.y));
    velocityTexel = texture2D(velocityMap, scaledPosUv);
  }

#ifdef TEXEL_TO_VELXY
  vec2 velocity = TEXEL_TO_VELXY(velocityTexel);
#else
  // Default image format presumes X and Y encoded in R and G, from -25 to +25 m/s
  vec2 velocity = mix(vec2(-25.0, -25.0), vec2(25.0, 25.0), velocityTexel.rg);
#endif

  // Scale up the velocity based on min/max values
  float distortion = cos(radians(posUV.y * 180.0 - 90.0));
  vec2 offset = vec2(velocity.x / distortion, velocity.y) * 0.0001 * speedFactor;
  position.x += offset.x;

  if (position.x >= 1.0) position.x -= 1.0;
  else if (position.x < 0) position.x += 1.0;

  position.y += offset.y;
  if (position.y >= 1.0) position.y -= 1.0;
  else if (position.y < 0) position.y += 1.0;

  // Store the speed in z, in meters per second
  position.z = length(velocity);
  out_particle = vec4(position, life);
}
