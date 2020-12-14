#version 330

// Very simple shader just needs to pass position along
void main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
