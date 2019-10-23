#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 colors;

out vec3 pos;
out vec3 col;

void main()
{
	col = colors;
	pos = vertPos;
	gl_Position = vec4(vertPos, 1.0);
}
