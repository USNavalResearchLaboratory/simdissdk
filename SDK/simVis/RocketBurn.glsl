#pragma vp_entryPoint sim_RocketBurn_VS
#pragma vp_location vertex_view

out vec2 sim_RocketBurn_texcoord;
in float sim_RocketBurn_radius; // attr 6
uniform mat4 osg_ViewMatrixInverse;

void sim_RocketBurn_VS(inout vec4 vertexView)
{
  // shortcut that assumes positive uniform scaling:
  float scale = length((osg_ViewMatrixInverse * gl_ModelViewMatrix)[0].xyz);

  // verts are in groups of 4 so take a modulus:
  int n = gl_VertexID & 3;

  // expand the 4 verts into a billboard in view space:
  float x = (n == 2 || n == 3) ? -1.0 : 1.0;
  float y = (n == 0 || n == 3) ? -1.0 : 1.0;
  vertexView.xy += vec2(x * sim_RocketBurn_radius * scale, y * sim_RocketBurn_radius * scale);

  // generate the texture coordinate:
  sim_RocketBurn_texcoord = vec2(x, y) * 0.5 + 0.5;
}

[break]

#pragma vp_entryPoint sim_RocketBurn_FS
#pragma vp_location fragment_coloring

in vec2 sim_RocketBurn_texcoord;
uniform sampler2D sim_RocketBurn_tex;
void sim_RocketBurn_FS(inout vec4 color)
{
  color *= texture(sim_RocketBurn_tex, sim_RocketBurn_texcoord);
}
