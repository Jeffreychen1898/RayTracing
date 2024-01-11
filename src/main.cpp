#include <iostream>
#include <chrono>
#include <cmath>
#include <random>
#include <cfloat>

#include <renderer/Renderer.hpp>

#include "Scene.hpp"

// renderer todo:
//		renderer.clear()
//		renderer throw exception on too large of a shape
//		adjust point size
//		vector get angle
//		shader must be bound when resizing window?
//		vector and matrix *=, +=, etc

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BATCH_COUNT 50

#define FOCAL_DISTANCE 400

static void randVec3(Renderer::Vec3<float>& _vec3);
static void renderPerPixel(int _x, int _y, float &_red, float &_green, float &_blue);

static std::vector<Sphere> scene_spheres;

struct Ray
{
	Renderer::Vec3<float> origin;
	Renderer::Vec3<float> direction;
};

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.f, 1.f);

int main()
{
	Renderer::Window::GLFWInit();

	Renderer::Window window;
	window.init(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Tracing");

	// create the renderer
	Renderer::Render renderer;
	renderer.attach(&window);
	renderer.init();

	// gl enable
	glPointSize(4.f);

	// create the shader
	Renderer::Shader shader;
	shader.attach(&window);
	shader.createFromFile("res/basic.vert", "res/basic.frag", true);
	shader.vertexAttribAdd(0, Renderer::AttribType::VEC2);
	shader.vertexAttribAdd(1, Renderer::AttribType::VEC3);
	shader.vertexAttribsEnable();
	shader.uniformAdd("u_projection", Renderer::UniformType::MAT4);
	Renderer::Mat4<float> projection_matrix = Renderer::Math::projection2D(
		0.f, static_cast<float>(WINDOW_WIDTH),
		0.f, static_cast<float>(WINDOW_HEIGHT),
		1.f, -1.f
	);
	shader.setUniformMatrix("u_projection", *projection_matrix);

	// add the shapes to the scene
	scene_spheres.push_back(Sphere(0.5f, 0.f, -0.1f, -4.f,		1.f, 0.5f, 0.5f,		0.f));
	scene_spheres.push_back(Sphere(10.f, 0.f, -11.f, -2.f,		0.5f, 0.5f, 1.f,	0.f));
	scene_spheres.push_back(Sphere(0.4f, 2.f, 0.f, -3.f,		0.9f, 0.9f, 0.9f,			20.f));
	scene_spheres.push_back(Sphere(0.3f, 2.f, 0.f, -5.f,		0.48f, 0.38f, 0.57f,			0.f));

	auto previous_time = std::chrono::high_resolution_clock::now();
	float frame_index = 0.f;

	float* pixel_data = new float[WINDOW_WIDTH * WINDOW_HEIGHT * 3];
	for(unsigned int i=0;i<WINDOW_WIDTH * WINDOW_HEIGHT * 3; ++i)
		pixel_data[i] = 0.f;

	while(window.isOpened())
	{
		// calculate the fps
		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - previous_time);
		previous_time = current_time;
		float dt = elapsed_time.count() * 1e-9;
		std::cout << 1.f / dt << "\n";

		++ frame_index;

		// clear buffer
		glClear(GL_COLOR_BUFFER_BIT);

		// draw a point on the screen
		renderer.bindShader(&shader);
		for(int b=0;b<BATCH_COUNT;++b)
		{
			// render each batch
			int render_height = std::ceil(WINDOW_HEIGHT / static_cast<float>(BATCH_COUNT));
			renderer.beginShape(Renderer::DrawType::POINTS, WINDOW_WIDTH*render_height, 0);

			for(int i=0;i<render_height;++i)
			{
				for(int j=0;j<WINDOW_WIDTH;++j)
				{
					int y_coord = i + render_height * b;

					float& px_red = pixel_data[3 * (WINDOW_WIDTH * y_coord + j)];
					float& px_green = pixel_data[3 * (WINDOW_WIDTH * y_coord + j) + 1];
					float& px_blue = pixel_data[3 * (WINDOW_WIDTH * y_coord + j) + 2];

					float red, green, blue;
					renderPerPixel(j, WINDOW_HEIGHT - y_coord, red, green, blue);

					px_red += red;
					px_green += green;
					px_blue += blue;

					if(!(i == 0 && j == 0))
						renderer.nextVertex();
					renderer.vertex2f(j, y_coord);
					renderer.vertex3f(px_red / frame_index, px_green / frame_index, px_blue / frame_index);
				}
			}
			
			renderer.endShape();
		}

		renderer.render();

		window.swapBuffers();
		Renderer::Window::pollEvents();
	}

	delete[] pixel_data;

	return 0;
}

void renderPerPixel(int _x, int _y, float &_red, float &_green, float &_blue)
{
	// set the default values
	_red = 0.f;
	_green = 0.f;
	_blue = 0.f;

	// cast the ray from the camera
	Ray ray = {
		Renderer::Vec3<float>(0.f, 0.f, 0.f),
		Renderer::Vec3<float>(_x - WINDOW_WIDTH / 2, _y - WINDOW_HEIGHT / 2, -FOCAL_DISTANCE)
	};

	float absorb_r = 1.f;
	float absorb_g = 1.f;
	float absorb_b = 1.f;

	for(int i=0;i<10;++i)
	{
		float closest_hit = FLT_MAX;
		int object_idx = -1;
		// sphere collisions
		for(int i=0;i<scene_spheres.size();++i)
		{
			float t = scene_spheres[i].findT(ray.origin, ray.direction);
			if(t <= 0)
				continue;

			if(closest_hit > t && t > 0.0000000001f)
			{
				closest_hit = t;
				object_idx = i;
			}
		}
		if(object_idx > -1)
		{
			Renderer::Vec3<float> sphere_normal = scene_spheres[object_idx].findNormal(closest_hit, ray.origin, ray.direction);
			ray.origin = ray.origin + ray.direction * closest_hit;

			Renderer::Vec3<float> rand_vec3;
			randVec3(rand_vec3);
			//ray.direction = (ray.direction * -1.f).reflect(sphere_normal) + rand_vec3;
			ray.direction = sphere_normal + rand_vec3;

			if(scene_spheres[object_idx].isLightSource())
			{
				scene_spheres[object_idx].addLights(_red, _green, _blue, absorb_r, absorb_g, absorb_b);
				return;
			}
			else
				scene_spheres[object_idx].multColors(absorb_r, absorb_g, absorb_b);

			continue;
		}

		_red += 0.627f * absorb_r;
		_green += 1.f * absorb_g;
		_blue += 1.f * absorb_b;

		return;
	}
}

void randVec3(Renderer::Vec3<float>& _vec3)
{
	_vec3.x = dis(gen) * 2.f - 1.f;
	_vec3.y = dis(gen) * 2.f - 1.f;
	_vec3.z = dis(gen) * 2.f - 1.f;

	_vec3.normalize();
}
