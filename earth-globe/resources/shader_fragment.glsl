#version 330 core
out vec3 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 frag_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform float f;

void main()
{
vec2 flipped_tex = vec2(1) - frag_tex;
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(-30,-40,-100);
vec3 ld = normalize(vertex_pos - lp);
float diffuse = dot(n,ld);
diffuse = clamp(diffuse,0,1);

color = texture(tex, flipped_tex).rgb;
vec3 colornight = texture(tex2, flipped_tex).rgb;
vec3 specmap = texture(tex3, flipped_tex).rgb;
vec3 clouds = texture(tex4, flipped_tex).rgb;

color = color * diffuse + colornight * (1-diffuse);
clouds = clouds * diffuse;

vec3 cd = normalize(vertex_pos - campos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,20);
color += vec3(1,1,1)*spec*specmap;
color += clouds.rgb;



}
