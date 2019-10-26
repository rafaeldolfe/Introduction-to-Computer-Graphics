#version 330 core
out vec4 color;

in vec3 fragTex;
in vec3 fragNor;
void main()
{
	vec3 normal = normalize(fragNor);
	color.rgb = normal + vec3(0.7f, 0.7f, 0.7f);
	color.a=1;
}
