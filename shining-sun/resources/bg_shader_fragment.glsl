#version 330 core
out vec4 color;

in vec3 pos;

void main()
{
	float dist = 1 - distance(pos, vec3(0.0f, 0.0f, 0.0f));

	if (dist > 0.80f)
	{
		color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else 
	{
		vec3 dark_blue = vec3(0.0f, 0.0f, 0.5f);
		vec3 blue = vec3(0.4f, 0.4f, 1.0f);
		vec3 mixed = mix(dark_blue, blue, dist);

		color = vec4(mixed, 1.0f);
	}

}
