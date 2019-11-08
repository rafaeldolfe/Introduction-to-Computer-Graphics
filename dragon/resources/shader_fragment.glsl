#version 330 core
out vec3 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 frag_tex;

uniform vec3 lightpos;
uniform vec3 campos;

uniform sampler2D tex;

void main()
{
vec2 corrected_tex = vec2(frag_tex.x, 1-frag_tex.y);
vec3 n = normalize(vertex_normal);
vec3 ld = normalize(vertex_pos - lightpos);
float diffuse = dot(n,ld);
diffuse = clamp(diffuse,0,1);

color = texture(tex, corrected_tex).rgb;
color = color * diffuse;

vec3 cd = normalize(vertex_pos - campos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,20);
color += vec3(1,1,1)*spec;




}
