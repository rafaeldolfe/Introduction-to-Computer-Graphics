#version 330 core
out vec4 color;

uniform vec3 colour;
uniform vec3 campos;

in vec3 vertex_color;
in vec3 vertex_pos;
void main()
{
	color.rgb = colour;
	color.a=1;	//transparency: 1 .. 100% NOT transparent
}
