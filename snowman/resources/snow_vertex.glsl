#version 330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float time;
uniform float direction;
void main()
{
	vec4 parametric_position = vec4(vertPos.x + 5*cos(time * direction), vertPos.y, vertPos.z + 5*sin(time * direction), 1);
	gl_Position = P * V * M * parametric_position;
}
