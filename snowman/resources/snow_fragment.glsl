#version 330 core
out vec4 color;
uniform float alpha;
void main()
{
	color.rgb = vec3(0.9f, 0.9f, 0.9f);
	color.a=alpha;
}
