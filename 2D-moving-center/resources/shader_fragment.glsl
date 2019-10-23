#version 330 core
out vec4 color;

uniform vec2 center;

in vec3 pos;
in vec3 col;
in vec2 centerPoint;

void main()
{
	
	if (pos.y > 0.0f)
		color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	float dist = distance(pos, vec3(center.x / 5,center.y / 5,0.0f));
	if (dist < 0.05f)
		discard;
	if (dist > 0.05f)
	{
		float newDist = 1.0 - dist * 3;

		float x = col.x;
		float y = col.y;
		float z = col.z;
		float a = newDist;
		color = vec4(x, y, z, a);
	}
}
