#version 410 core

precision highp float;

in vec2 v_texCoord;
in vec2 v_rayCoord;

uniform sampler2D u_randTexture;
uniform vec2 u_randSampler;

uniform vec3 u_cameraPosition;
uniform vec3 u_cameraForward;
uniform vec3 u_cameraUp;
uniform float u_focalDistance;
uniform float u_metallic;
uniform float u_roughness;

uniform vec3 u_skyColor;
uniform float u_skyIntensity;

layout(location = 0) out vec4 FragColor;

#define PI 3.14159265f
#define EP 0.00000001f

// need
// 
// albedo
// V: ray_direction
// L: scatter_direction
// N: normal
// roughness
// H: normalize(V + L)
// reflectivity
// metallic
float GGXDistributionFunction(float _alpha, vec3 _N, vec3 _H)
{
	float numerator = _alpha * _alpha;
	float denominator = PI * pow(pow(max(dot(_N, _H), 0.f), 2) * (_alpha * _alpha - 1.f) + 1.f, 2);
	return numerator / max(denominator, EP);
}

float geoShadowFunction(float _alpha, vec3 _L, vec3 _N, vec3 _V)
{
	float k = _alpha / 2.f;
	float G1 = max(dot(_N, _L), 0.f) / max(max(dot(_N, _L), 0.f) * (1.f - k) + k, EP);
	float G2 = max(dot(_N, _V), 0.f) / max(max(dot(_N, _V), 0.f) * (1.f - k) + k, EP);
	return G1 * G2;
}

vec4 brdf(vec3 _albedo, float _roughness, float _metallic, vec3 _reflectivity, vec3 _V, vec3 _L, vec3 _N)
{
	vec3 H = normalize(_V + _L);

	// calculate ks and kd
	vec3 ks = _reflectivity + (1.f - _reflectivity) * pow(1.f - max(dot(_V, H), 0.f), 5);
	vec3 kd = (1.f - ks) * (1.f - _metallic);

	// calculate diffuse light
	vec3 diffuse = _albedo * kd / PI;

	// calculate specular light (cook - torrance)
	float alpha = _roughness * _roughness;
	float k = alpha / 2.f;
	float D = GGXDistributionFunction(alpha, _N, H);
	float G = geoShadowFunction(alpha, _L, _N, _V);

	float part_cook_torrance = D * G / max(4.f * max(dot(_V, _N), 0.f) * max(dot(_L, _N), 0.f), EP);

	vec3 cook_torrance = ks * part_cook_torrance;
	float weight = max(dot(_N, _L), 0.f) / PI;
	//float metal_weight = D * G / max(4.f * max(dot(_V, _N), 0.f) * max(dot(_L, _N), 0.f), EP);
	float metal_weight = D;
	weight = mix(weight, metal_weight, _metallic);

	// calculate L dot N
	return vec4((diffuse + cook_torrance) * max(dot(_N, _L), 0.f), weight);
}

vec3 lambertianSampling(vec3 _normal, vec3 _randVec3)
{
	return normalize(_normal + _randVec3 * 0.99f);
}

vec3 metallicSampling(vec3 _normal, vec3 _rayDir, float _roughness, vec3 _randVec3)
{
	vec3 reflected_ray = reflect(_rayDir, _normal);
	vec3 new_ray = reflected_ray + _randVec3 * _roughness * 0.5f;

	if(dot(new_ray, _normal) > 0.f)
		return new_ray;
	return reflected_ray;
}

void main()
{
	const float anti_alias_val = 0.9f;
	const int sphere_count = 4;
	const int ray_bounce = 6;

	// might be a better way to randomize
	vec2 tex_coord = v_texCoord + u_randSampler;
	vec4 rand_vec = texture(u_randTexture, tex_coord);
	rand_vec = (rand_vec - vec4(0.5f)) * vec4(2.f);

	float radius[sphere_count] = float[](0.5f, 30.f, 10.f, 0.25f);
	vec3 positions[sphere_count] = vec3[](vec3(0.f, -2.5f, -5.f), vec3(0.f, -33.f, -5.f), vec3(10.f, 10.f, -50.f), vec3(-.5f, -2.5f, -4.f));
	vec3 color[sphere_count] = vec3[](vec3(0.7216f, 0.4510f, 0.2f), vec3(0.9f, 0.9f, 0.9f), vec3(1.f, 1.f, 1.f), vec3(0.5f, 0.5f, 1.f));
	float brightness[sphere_count] = float[](0.f, 0.f, 10.f, 5.f);

	vec3 camera_forward = normalize(u_cameraForward);
	vec3 camera_right = cross(camera_forward, normalize(u_cameraUp));
	vec3 camera_up = cross(camera_right, camera_forward);

	vec3 ray_origin = u_cameraPosition;
	vec3 ray_direction = u_focalDistance * camera_forward + v_rayCoord.x * camera_right + v_rayCoord.y * camera_up + anti_alias_val * vec3(rand_vec.xy, 0);
	ray_direction = normalize(ray_direction);

	vec3 absorb_col = vec3(1.f);
	vec3 light_col = vec3(0.f);
	float weight = 1.f;

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
			absorb_col = clamp(absorb_col, vec3(0.0f), vec3(1.f));
			//light_col += vec3(0.627f, 1.f, 1.f) * absorb_col * vec3(0.4f);
			//light_col += vec3(0.f);
			light_col += u_skyColor * u_skyIntensity * absorb_col;
			break;
		}

		vec3 surface = ray_origin + ray_direction * closest_hit;
		vec3 normal = normalize(surface - positions[idx]);
		vec3 view_vector = -ray_direction;

		ray_origin = surface + normal * 0.00000000001f;
		//ray_direction = normalize(normal + rand_vec.xyz * 0.99f);
		//ray_direction = reflect(ray_direction, normal);
		
		float roughness = u_roughness;
		float metallic = u_metallic;
		
		vec3 metallic_sample = metallicSampling(normal, ray_direction, roughness, rand_vec.xyz);
		vec3 lambertian_sample = lambertianSampling(normal, rand_vec.xyz);
		ray_direction = mix(lambertian_sample, metallic_sample, metallic);

		absorb_col = clamp(absorb_col, vec3(0.f), vec3(1.f));
		light_col += color[idx] * absorb_col * brightness[idx];

		vec4 brdf_scattering = brdf(vec3(1.f), roughness, metallic, vec3(1.f), normalize(view_vector), normalize(ray_direction), normal);
		brdf_scattering = clamp(brdf_scattering, vec4(0.f), vec4(1.f));
		absorb_col *= brdf_scattering.rgb * color[idx];
		weight *= brdf_scattering.a;
	}

	weight = clamp(weight, 0.f, 1.f);
	light_col = clamp(light_col, vec3(0.f), vec3(1.f));
	FragColor = vec4(light_col, weight);
}
