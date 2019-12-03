#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout (location = 3) in vec4 InstancePos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform float w;
out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;
void main()
{
	float rot = w / length(InstancePos);
	mat4 rotation = mat4(vec4(cos(rot), 0, sin(rot), 0), vec4(0, 1, 0, 0), vec4(-sin(rot), 0, cos(rot), 0), vec4(0, 0, 0, 0));
	vertex_normal = vec4(M * vec4(vertNor,0.0)).xyz;
	vec4 pos = M * vec4(vertPos,1.0) + InstancePos * rotation;
	gl_Position = P * V * pos;
	vertex_tex = vertTex;	
}
