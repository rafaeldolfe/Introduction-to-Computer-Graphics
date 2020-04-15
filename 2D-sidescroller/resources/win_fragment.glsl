#version 330 core
out vec4 color;

uniform vec3 colour;
uniform sampler2D tex;

in vec3 vertex_color;
in vec2 vertex_tex;
void main()
{
	vec4 tcol = texture(tex, vertex_tex);
	color = tcol;
	color.a = 1.0f;
}
