#version 330 core
out vec4 color;

uniform float transparency;

in vec3 fragTex;
in vec3 fragNor;
in vec3 pos;
void main()
{
	vec3 normal = normalize(fragNor);
	color.rgb = vec3(0.7f, 0.7f, 0.7f) - vec3(0.3) * (1 - (pos.z - 5));
	color.a=1-(pos.z - 5)*1.5f;
}
