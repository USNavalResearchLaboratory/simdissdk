#version 330

#pragma vp_location vertex_view
#pragma vp_entryPoint simVis_flatten_VS

vec3 vp_Normal;
uniform mat4 osg_ViewMatrixInverse;
uniform mat4 osg_ViewMatrix;
uniform bool simVis_useFlattenShader;

// Define a helper method to pull out the earth radius given the sin of latitude
float simVis_EarthRadius(in float sinLat)
{
  float cosLat = cos(asin(sinLat));
  const float a = $EARTH_RADIUS_MAJOR;
  const float b = $EARTH_RADIUS_MINOR;
  float top = pow(a * a * cosLat, 2.0) + pow(b * b * sinLat, 2.0);
  float bottom = pow(a * cosLat, 2.0) + pow(b * sinLat, 2.0);
  // Note that divide-by-zero is impossible because sin(lat) and cos(lat) cannot both be 0 simultaneously
  return sqrt(top / bottom);
}

void simVis_flatten_VS(inout vec4 viewVertex)
{
  if (simVis_useFlattenShader) {
    vec4 worldVertex = osg_ViewMatrixInverse * viewVertex;
    const float a = $EARTH_RADIUS_MAJOR;
    const float b = $EARTH_RADIUS_MINOR;

    vec3 up = normalize(worldVertex.xyz);
    float R = simVis_EarthRadius(up.z);

    viewVertex.xyz = up * R;
    viewVertex = osg_ViewMatrix * viewVertex;
  }
}
