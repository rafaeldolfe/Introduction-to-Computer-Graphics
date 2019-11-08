#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform vec3 campos;
uniform vec2 cloud_n;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform sampler2D tex3;

void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(10,-20,-100);
vec3 ld = normalize(vertex_pos - lp);
float diffuse = dot(n,ld);

vec2 texcoords = vertex_tex/4 + cloud_n/4;

vec4 tcol = texture(tex3, texcoords);
color = tcol;
color.a = (tcol.r + tcol.g + tcol.b) / 3;



}
