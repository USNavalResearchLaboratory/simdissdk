#version $GLSL_VERSION_STR

#pragma vp_entryPoint simutil_vpl_rtt_vertex
#pragma vp_location   vertex_model

uniform sampler2D positionSampler;
uniform sampler2D directionSampler;
uniform vec2 resolution;
uniform float pointSize;
uniform float altitude;

uniform mat4 osg_ViewMatrix;

out vec4 particle_color;
out float rotate_angle;

const float PI = 3.1415926535897932384626433832795;
const float PI_2 = 1.57079632679489661923;
const float TWO_PI = 6.283185307179586476925286766559;

// Color mapping function for velocity to color.  VelocityParticleLayer supplies this programmatically from gradient option.
vec4 su_vel2color(in float value);

// Helper function to convert LLA to ECEF XYZ
vec3 convertLatLongHeightToXYZ(float latitude, float longitude, float height)
{
  float radiusEquator = 6378137.0;
  float radiusPolar = 6356752.3142;
  float flattening = (radiusEquator - radiusPolar) / radiusEquator;
  float eccentricitySquared = 2 * flattening - flattening * flattening;
  float sin_latitude = sin(latitude);
  float cos_latitude = cos(latitude);
  float N = radiusEquator / sqrt(1.0 - eccentricitySquared * sin_latitude*sin_latitude);
  float X = (N + height)*cos_latitude*cos(longitude);
  float Y = (N + height)*cos_latitude*sin(longitude);
  float Z = (N*(1 - eccentricitySquared) + height)*sin_latitude;
  return vec3(X,Y,Z);
}

float speedToScale(float speedMs)
{
  return min(speedMs / 25.0, 1.0);
}

// Calculates the position, color, and angle for particles
void simutil_vpl_rtt_vertex(inout vec4 vertexModel)
{
  // Using the instance ID, generate texture coords for this instance.
  vec2 tC;
  float r = float(gl_InstanceID) / resolution.x;
  tC.s = fract( r ); tC.t = floor( r ) / resolution.y;

  // Use the (scaled) tex coord to translate the position of the vertices.
  vec4 posInfo = texture2D( positionSampler, tC );
  float life = posInfo.w;
  float velocity = posInfo.z;

  particle_color = su_vel2color(velocity);
  // Rotation angle comes from the direction sampler texture
  rotate_angle = texture2D(directionSampler, tC).r;

  // Compute the heading based on the view matrix that will orient the rotation angle correctly based on the camera orientation.
  vec3 upVector = vec3(osg_ViewMatrix[2][0], osg_ViewMatrix[2][1], osg_ViewMatrix[2][2]);
  normalize(upVector);
  // Compute azimuth offset from eye using arc tangent
  float heading = atan(upVector.x, upVector.y);
  rotate_angle -= heading;

  vec3 lla = vec3(-PI + TWO_PI * posInfo.x, -PI_2 + posInfo.y * PI, altitude);
  vec3 xyz = convertLatLongHeightToXYZ(lla.y, lla.x, lla.z);
  vertexModel.xyz = xyz;

  // Scale the point size higher if rendering points, to account for circular radius blending
  gl_PointSize = pointSize;
}
