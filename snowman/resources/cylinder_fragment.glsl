#version 330 core
out vec4 color;
in vec3 fragNor;
void main()
{
	vec3 normal = normalize(fragNor);
	color.rgb = normal * 0.15f + vec3(0.3f, 0.3f, 0.3f);
	color.a=1;
}
