#version 330 core
out vec4 color;

in vec3 pos;

void main()
{
	float dist = distance(pos, vec3(0.0f, 0.0f, 0.0f));
	if (dist < 0.20f)
	{
		color = vec4(0.98f, 0.75f, 0.12f, 1 - dist * 3);
	}
	else if (dist > 0.20f)
	{
		float newDist = 1.0 - dist;

		vec3 orange_red = vec3(1.0f, 0.12f, 0.0f);
		vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
		vec3 mixed = mix(orange_red, yellow, newDist);

		color = vec4(mixed, 1.0f);
	}
}
