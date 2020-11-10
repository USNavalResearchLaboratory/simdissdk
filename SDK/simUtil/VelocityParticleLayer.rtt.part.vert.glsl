#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma vp_entryPoint simutil_vpl_vertex
#pragma vp_location   vertex_model
#pragma import_defines(RTT_OUTPUT_IMAGE_WIDTH, RTT_OUTPUT_IMAGE_HEIGHT)

uniform sampler2D positionSampler;
uniform vec2 resolution;
uniform float pointSize;
out vec4 particle_color;
out float rotate_angle;

void simutil_vpl_vertex(inout vec4 vertexModel)
{
  // Using the instance ID, generate texture coords for this instance.
  vec2 tC;
  float r = float(gl_InstanceID) / resolution.x;
  tC.s = fract( r ); tC.t = floor( r ) / resolution.y;

  // Use the (scaled) tex coord to translate the position of the vertices.
  vec4 posInfo = texture2D( positionSampler, tC );
  float life = posInfo.w;

  particle_color = mix(vec4(1.0, 0.0, 0.0, 0.3), vec4(0.0, 1.0, 0.0, 1.0), life);
  rotate_angle = 0.0; // Unused

  vec3 pos = posInfo.xyz;
  pos.xy *= vec2(RTT_OUTPUT_IMAGE_WIDTH, RTT_OUTPUT_IMAGE_HEIGHT);
  vertexModel.xyz = pos;

  gl_PointSize = pointSize;
}
