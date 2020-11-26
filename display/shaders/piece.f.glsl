#version 420 core

in vec4 pos;
in vec3 norm;
in vec3 col;

out vec4 fcol;

uniform vec3 lpos;
uniform mat4 view_inv;

const vec3 lcol = vec3(1.0, 1.0, 1.0);

vec3 lighting(vec3 col0, vec3 norm, float ambient, float diffuse, float specular, float shininess)
{
  vec3 a = ambient * lcol;

  vec3 lv = normalize(lpos - pos.xyz);

  vec3 d = max(0, dot(lv, norm)) * diffuse * lcol;

  vec3 camera = (view_inv * vec4(0, 0, 0, 1)).xyz;
  vec3 rlv = reflect(-lv, norm);
  vec3 viewv = normalize(camera - pos.xyz);
  vec3 s = specular * pow(max(0, dot(rlv, viewv)), shininess) * lcol;

  return (a + d + s) * col0;
}

void main()
{
  vec3 col0 = lighting(col, norm, 0.3, 0.5, 0.0, 5.0);
  fcol = vec4(col0, 1);
}
