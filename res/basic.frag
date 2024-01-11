#version 410 core

in vec2 v_texCoord;
in vec2 v_rayCoord;

uniform sampler2D u_randTexture;
uniform float u_randSampler;

out vec4 FragColor;

vec3 proj(vec3 _vec, vec3 _on)
{
	return dot(_vec, _on) / dot(_on, _on) * _on;
}

vec3 refl(vec3 _vec, vec3 _against)
{
	return 2 * proj(_vec, _against) - _vec;
}

void main()
{
	float window_width = 800.f;
	float window_height = 600.f;
	float focal_distance = -400.f;

	const int sphere_count = 4;
	const int ray_bounce = 8;

	// might be a better way to randomize
	vec2 tex_coord = v_texCoord + vec2(u_randSampler, 0.f);
	vec4 rand_vec = texture(u_randTexture, tex_coord);
	rand_vec = (rand_vec - vec4(0.5f)) * vec4(2.f);

	float radius[sphere_count] = float[](0.5f, 10.f, 0.4f, 0.3f);
	vec3 positions[sphere_count] = vec3[](vec3(0.f, -0.7f, -4.f), vec3(0.f, -11.f, -2.f), vec3(2.f, 0.f, -3.f), vec3(2.f, 0.f, -5.f));
	vec3 color[sphere_count] = vec3[](vec3(1.f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 1.f), vec3(0.9f, 0.9f, 0.9f), vec3(0.48f, 0.38f, 0.57f));
	float brightness[sphere_count] = float[](0.f, 0.f, 20.f, 0.f);

	vec3 ray_origin = vec3(0.f, 0.f, 0.f);
	vec3 ray_direction = vec3(v_rayCoord.x, v_rayCoord.y, focal_distance);

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
			if(t > 0.f && t < closest_hit)
			{
				closest_hit = t;
				idx = j;
			}
		}

		if(idx < 0)
		{
			// ensure absorb color is between 0 and 1, no idea why code breaks when i remove this.
			absorb_col = clamp(absorb_col, vec3(0.f), vec3(1.f));
			//light_col += vec3(0.627f, 1.f, 1.f) * absorb_col;
			light_col += vec3(0.f);
			break;
		}

		vec3 surface = ray_origin + ray_direction * closest_hit;
		vec3 normal = normalize(surface - positions[idx]);
		vec3 incident_vector = normalize(-ray_direction);

		ray_origin = surface + normal * 0.00000000001f;
		//ray_direction = refl(incident_vector, normal);
		ray_direction = normal + rand_vec.xyz;

		if(brightness[idx] > 0.f)
		{
			light_col += color[idx] * absorb_col * brightness[idx];
			break;
		} else
		{
			absorb_col *= color[idx];
		}
	}

	FragColor = vec4(light_col, 1.0);
}
