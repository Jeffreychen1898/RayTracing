#version 410 core

in vec2 v_texCoord;
in vec2 v_rayCoord;

uniform sampler2D u_randTexture;
uniform vec2 u_randSampler;

uniform vec3 u_cameraPosition;
uniform vec3 u_cameraForward;
uniform vec3 u_cameraUp;
uniform float u_focalDistance;

out vec4 FragColor;

void main()
{
	const int sphere_count = 4;
	const int ray_bounce = 10;

	// might be a better way to randomize
	vec2 tex_coord = v_texCoord + u_randSampler;
	vec4 rand_vec = texture(u_randTexture, tex_coord);
	rand_vec = (rand_vec - vec4(0.5f)) * vec4(2.f);

	float radius[sphere_count] = float[](0.5f, 100.f, 80.f, 0.1f);
	vec3 positions[sphere_count] = vec3[](vec3(0.f, -2.f, -5.f), vec3(0.f, -103.f, -5.f), vec3(100.f, 80.f, -100.f), vec3(-0.8f, -1.7f, -4.f));
	vec3 color[sphere_count] = vec3[](vec3(1.f, 0.5f, 0.5f), vec3(0.5f, 0.5f, 1.f), vec3(0.9f, 0.9f, 0.3f), vec3(0.48f, 0.38f, 0.57f));
	float brightness[sphere_count] = float[](0.f, 0.f, 10.f, 500.f);

	vec3 camera_forward = normalize(u_cameraForward);
	vec3 camera_right = cross(camera_forward, normalize(u_cameraUp));
	vec3 camera_up = cross(camera_right, camera_forward);

	vec3 ray_origin = u_cameraPosition;
	vec3 ray_direction = u_focalDistance * camera_forward + v_rayCoord.x * camera_right + v_rayCoord.y * camera_up;

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
			//light_col += vec3(0.627f, 1.f, 1.f) * vec3(0.3) * absorb_col;
			light_col += vec3(0.f);
			break;
		}

		vec3 surface = ray_origin + ray_direction * closest_hit;
		vec3 normal = normalize(surface - positions[idx]);
		vec3 incident_vector = normalize(-ray_direction);

		ray_origin = surface + normal * 0.00000000001f;
		ray_direction = normal + rand_vec.xyz * 0.99f;

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
