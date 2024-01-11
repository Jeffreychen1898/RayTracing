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
	shader.uniformAdd("u_randTexture", Renderer::UniformType::INT);
	shader.uniformAdd("u_randSampler", Renderer::UniformType::FLOAT);
	Renderer::Mat4<float> projection_matrix = Renderer::Math::projection2D(
		0.f, static_cast<float>(WINDOW_WIDTH),
		0.f, static_cast<float>(WINDOW_HEIGHT),
		1.f, -1.f
	);
	shader.setUniformMatrix("u_projection", *projection_matrix);
	shader.setUniformInt("u_randTexture", 0);
	shader.setUniformFloat("u_randSampler", 0.f);

	// create the random normal vector texture
	unsigned char* rand_vec_data = new unsigned char[WINDOW_WIDTH * WINDOW_HEIGHT * 3];
	for(unsigned int i=0;i<WINDOW_WIDTH * WINDOW_HEIGHT;++i)
	{
		Renderer::Vec3<float> rand_vector;
		randVec3(rand_vector);

		rand_vec_data[i * 3] = static_cast<unsigned char>(rand_vector.x * 255);
		rand_vec_data[i * 3 + 1] = static_cast<unsigned char>(rand_vector.y * 255);
		rand_vec_data[i * 3 + 2] = static_cast<unsigned char>(rand_vector.z * 255);
	}

	float random_sampler = 0.f;

	Renderer::Texture random_vectors(8);
	random_vectors.setTextureFilter(GL_NEAREST, GL_NEAREST);
	random_vectors.setTextureWrap(GL_REPEAT, GL_REPEAT);
	random_vectors.create(&window, WINDOW_WIDTH, WINDOW_HEIGHT, 3, rand_vec_data);
	delete[] rand_vec_data;

	auto previous_time = std::chrono::high_resolution_clock::now();

	while(window.isOpened())
	{
		renderer.bindTexture(&random_vectors);
		// calculate the fps
		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - previous_time);
		previous_time = current_time;
		float dt = elapsed_time.count() * 1e-9;
		std::cout << 1.f / dt << "\n";

		random_sampler = dis(gen) * WINDOW_WIDTH;
		shader.setUniformFloat("u_randSampler", random_sampler);

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

					if(!(i == 0 && j == 0))
						renderer.nextVertex();
					renderer.vertex2f(j, y_coord);
					renderer.vertex3f(1.f, 1.f, 1.f);
				}
			}
			
			renderer.endShape();
		}

		renderer.render();

		window.swapBuffers();
		Renderer::Window::pollEvents();
	}

	return 0;
}

void randVec3(Renderer::Vec3<float>& _vec3)
{
	_vec3.x = dis(gen) * 2.f - 1.f;
	_vec3.y = dis(gen) * 2.f - 1.f;
	_vec3.z = dis(gen) * 2.f - 1.f;

	_vec3.normalize();
}
