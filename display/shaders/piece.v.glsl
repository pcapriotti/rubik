#version 430 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in int f;
layout (location = 3) in vec3 b;
layout (location = 4) in uint s;
layout (location = 5) in uint s0;

layout (std140, binding = 0) uniform scene_data
{
  mat4 view;
  mat4 view_inv;
  mat4 proj;
  mat4 model;
  vec3 lpos;
};

layout (std430, binding = 1) buffer _syms
{
  mat4 syms[];
};

layout (std430, binding = 2) buffer _face_action
{
  uint face_action[];
};

out vec4 pos;
out vec3 norm;
out vec3 col;
out vec3 bary;

vec3 colours[12] = {
  vec3(1.0, 1.0, 1.0), // white
  vec3(0.4, 0.4, 0.4), // grey
  vec3(1.0, 1.0, 0.0), // yellow
  vec3(0.91, 0.85, 0.68), // pale yellow
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
  // mat4 sym = mat4(1);
  // sym[3].x = float(s) / 3;
  mat4 sym = model * syms[s];

  pos = sym * vec4(p, 1);
  gl_Position = proj * view * pos;
  norm = mat3(sym) * n;
  if (f < 0)
    col = bcol;
  else
    col = colours[face_action[s0 * 12 + f]];

  bary = b;
}
