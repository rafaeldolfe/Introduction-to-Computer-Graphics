#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertColor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 vertex_color;
out vec2 vertex_tex;
void main()
{
	vertex_tex = vertTex;
	vertex_color = vertColor;
	gl_Position = P * V * M * vec4(vertPos, 1.0);
}
