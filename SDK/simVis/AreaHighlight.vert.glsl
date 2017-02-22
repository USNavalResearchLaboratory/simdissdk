#version 110

#pragma vp_entryPoint simvis_areahighlight_vert
#pragma vp_location vertex_model
#pragma vp_order 3.0

uniform float osg_FrameTime;
uniform float simvis_areahighlight_scale;

void simvis_areahighlight_vert(inout vec4 vertex)
{
  // Rotation around local Z-axis
  float scaledFreq = osg_FrameTime * $ROTATE_FREQUENCY;
  float cosine = cos(scaledFreq);
  float sine = sin(scaledFreq);
  // Simplified version of the basic Rotation matrix
  float tempX = simvis_areahighlight_scale * ((cosine * vertex.x) + (-sine * vertex.y));
  vertex.y = simvis_areahighlight_scale * ((sine * vertex.x) + (cosine * vertex.y));
  vertex.x = tempX;
  // Adjust for Z-fighting with terrain
  vertex.z += 0.001;
}
