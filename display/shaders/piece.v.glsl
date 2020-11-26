#version 420 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in int f;
layout (location = 3) in vec3 b;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 pos;
out vec3 norm;
out vec3 col;
out vec3 bary;

vec3 colours[12] = {
  vec3(1.0, 1.0, 1.0), // white
  vec3(0.4, 0.4, 0.4), // grey
  vec3(1.0, 1.0, 0.0), // yellow
  vec3(1.0, 1.0, 0.92), // pale yellow
  vec3(0.5, 0.0, 0.5), // purple
  vec3(1.0, 0.75, 0.8), // pink
  vec3(0.0, 0.6, 0.0), // green
  vec3(0.2, 0.8, 0.2), // lime
  vec3(1.0, 0.0, 0.0), // red
  vec3(1.0, 0.65, 0.0), // orange
  vec3(0.0, 0.0, 1.0), // blue
  vec3(0.0, 0.5, 0.5), // teal
};

vec3 bcol = vec3(0.05, 0.05, 0.05);

void main()
{
  pos = model * vec4(p, 1);
  gl_Position = proj * view * pos;
  norm = mat3(model) * n;
  if (f < 0)
    col = bcol;
  else
    col = colours[f];
  bary = b;
}
