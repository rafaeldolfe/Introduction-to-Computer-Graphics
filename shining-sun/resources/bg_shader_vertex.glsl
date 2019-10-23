#version 330 core
layout(location = 0) in vec3 vertPos;

out vec3 pos;

void main()
{
	gl_Position = vec4(vertPos, 1.0);
	pos = vertPos;
}
