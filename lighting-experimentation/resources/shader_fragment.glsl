#version 330 core
out vec3 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
uniform vec3 campos;
uniform vec3 baseColor;
uniform vec3 globalLight;
uniform float specPowerFactor;
uniform float specFactor;
uniform float diffuseFactor;
void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(-500,-500,-1000);
vec3 ld = normalize(vertex_pos - lp);
float diffuse = dot(n,ld);
color = globalLight + baseColor*diffuse*diffuseFactor;

vec3 cd = normalize(vertex_pos - campos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,specPowerFactor);
color += vec3(1,1,1)*spec*specFactor;

}
