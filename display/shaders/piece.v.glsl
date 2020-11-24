#version 420 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 pos;
out vec3 norm;

void main()
{
  pos = model * vec4(p, 1);
  gl_Position = proj * view * pos;
  norm = n;
}
