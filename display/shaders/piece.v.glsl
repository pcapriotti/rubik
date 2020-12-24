#version 430 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in int f;
layout (location = 3) in vec3 b;
layout (location = 4) in uint index;
layout (location = 5) in vec4 q;

uniform float duration;

layout (std140, binding = 0) uniform scene_data
{
  mat4 view;
  mat4 view_inv;
  mat4 proj;
  mat4 model;
  vec3 lpos;
  float time;
};

layout (std430, binding = 1) buffer _facelet
{
  uint num_colours;
  uint facelet[];
};

layout (std430, binding = 2) buffer _colours
{
  vec3 colours[];
};

out vec4 pos;
out vec3 norm;
out vec3 col;
out vec3 bary;

vec3 bcol = vec3(0.05, 0.05, 0.05);

vec3 quat_mul(vec4 q, vec3 v)
{
  vec3 t = 2 * cross(q.xyz, v);
  return v + q.w * t + cross(q.xyz, t);
}

void main()
{
  pos = model * vec4(quat_mul(q, p), 1);
  gl_Position = proj * view * pos;
  norm = mat3(model) * quat_mul(q, n);
  if (f < 0)
    col = bcol;
  else
    col = colours[facelet[index * num_colours + f]];

  bary = b;
}
