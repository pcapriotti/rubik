#version 430 core
layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in int f;
layout (location = 3) in vec3 b;
layout (location = 4) in uint s;
layout (location = 5) in uint s0;
layout (location = 6) in uint s1;
layout (location = 7) in float tm0;

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

layout (std430, binding = 1) buffer _syms
{
  vec4 syms[];
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

vec3 quat_mul(vec4 q, vec3 v)
{
  vec3 t = 2 * cross(q.xyz, v);
  return v + q.w * t + cross(q.xyz, t);
}

vec4 quat_normalize(vec4 q)
{
  return q / sqrt(pow(length(q.xyz), 2) + pow(q[3], 2));
}


#define SLERP_EPS 0.0001
vec4 quat_slerp(vec4 q1, vec4 q2, float t)
{
  float d = dot(q1.xyz, q2.xyz) + q1.w * q2.w;
  if (d < 0) {
    d = -d;
    q2 = -q2;
  }

  if (d > 1 - SLERP_EPS) {
    return quat_normalize(mix(q1, q2, t));
  }

  float s = sqrt(1 - d * d);
  float theta = atan(s, d);
  float a = sin(theta * (1 - t)) / s;
  float b = sin(theta * t) / s;
  return a * q1 + b * q2;
}

void main()
{
  vec4 q = syms[s];
  if (s1 != s) {
    q = quat_slerp(q, syms[s1], (time - tm0) / duration);
  }

  pos = model * vec4(quat_mul(q, p), 1);
  gl_Position = proj * view * pos;
  norm = mat3(model) * quat_mul(q, n);
  if (f < 0)
    col = bcol;
  else
    col = colours[face_action[s0 * 12 + f]];

  if (s == 0) col = vec3(0.5, 0, 0);
  else col = vec3(0.7, 0.7, 0.7);

  bary = b;
}
