#version 410 core
precision highp float;

in vec3 v_color;
in vec2 v_screenPos;

uniform sampler2D u_randTexture;

out vec4 FragColor;

void main()
{
	vec2 tex_coord = gl_FragCoord.xy / vec2(1600.f, 1200.f);
	vec4 rand_vec = texture(u_randTexture, tex_coord);
	
	float window_width = 800.f;
	float window_height = 600.f;
	float focal_distance = -400.f;

	const int sphere_count = 3;
	const int ray_bounce = 2;

	float radius[sphere_count] = float[](0.5f, 10.f, 0.4f);
	vec3 positions[sphere_count] = vec3[](vec3(0.f, -0.1f, -4.f), vec3(0.f, -11.f, -2.f), vec3(2.f, 0.f, -3.f));
	vec3 color[sphere_count] = vec3[](vec3(1.f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 1.f), vec3(0.9f, 0.9f, 0.9f));
	float brightness[sphere_count] = float[](0.f, 0.f, 20.f);

	vec3 ray_origin = vec3(0.f, 0.f, 0.f);
	vec3 ray_direction = vec3(v_screenPos.x - window_width / 2.f, v_screenPos.y - window_height / 2.f, focal_distance);

	vec3 absorb_col = vec3(1.f);
	vec3 light_col = vec3(0.f);

	for(int i=0;i<ray_bounce;++i)
	{
		float closest_hit = 10000.f; // arbitrary large number
		int idx = -1;
		// loop through all the different objects
		for(int j=0;j<sphere_count;++j)
		{
			vec3 origin_to_center = ray_origin - positions[j];
			float a = dot(ray_direction, ray_direction);
			float b = 2.f * dot(origin_to_center, ray_direction);
			float c = dot(origin_to_center, origin_to_center) - radius[j] * radius[j];
			float discriminant = b * b - 4.f * a * c;
			if(discriminant < 0.f)
				continue;

			float t = (-b - sqrt(discriminant)) / (2.f * a);
			if(t > 0.00001f && t < closest_hit)
			{
				closest_hit = t;
				idx = j;
			}
		}

		if(idx < 0)
		{
			light_col += vec3(0.627f, 1.f, 1.f) * absorb_col[idx];
			break;
		}

		vec3 surface = ray_origin + ray_direction * closest_hit;
		vec3 normal = normalize(surface - positions[idx]);

		ray_origin = surface;
		ray_direction = reflect(normal, normalize(ray_direction));

		light_col = ray_direction;
	}

	FragColor = vec4(light_col, 1.0);
}
