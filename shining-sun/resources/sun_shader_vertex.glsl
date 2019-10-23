#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 5) in uint index;
layout(location = 6) in vec2 direction;

uniform float speed;

out vec3 pos;

void main()
{
	pos = vertPos;
	if (bool(index))
	{
		gl_Position = vec4(vertPos.x + direction.x * speed * index, vertPos.y + direction.y * speed * index, vertPos.z, 1.0);
		pos = vec3(vertPos.x + direction.x * speed * index, vertPos.y + direction.y * speed * index, vertPos.z);
	}
	else 
	{
		gl_Position = vec4(vertPos, 1.0);
		pos = vertPos;
	}
}
