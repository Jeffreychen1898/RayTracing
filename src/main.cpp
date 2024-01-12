#include <iostream>
#include <chrono>
#include <cmath>
#include <random>
#include <cfloat>
#include <unordered_set>

#include <renderer/Renderer.hpp>

#include "Scene.hpp"

// renderer todo:
//		renderer.clear()
//		renderer throw exception on too large of a shape
//		adjust point size
//		vector get angle
//		shader must be bound when resizing window?
//		vector and matrix *=, +=, etc

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define FOCAL_DISTANCE 400

class WindowEvents : public Renderer::WindowEvents
{
	void KeyPressed(int _key, int _scancode, int _mods) override
	{
		std::cout << _key << " pressed!\n";
	}

	void KeyReleased(int _key, int _scancode, int _mods) override
	{
		std::cout << _key << " released\n";
	}
};

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

	WindowEvents* win_evt = new WindowEvents;
	Renderer::Window window;
	window.init(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Tracing");
	window.addEvents(win_evt);
	window.setVSync(false);

	// create the renderer
	Renderer::Render renderer;
	renderer.attach(&window);
	renderer.init();

	// create the shader
	Renderer::Shader shader;
	shader.attach(&window);
	shader.createFromFile("res/basic.vert", "res/basic.frag", true);
	shader.vertexAttribAdd(0, Renderer::AttribType::VEC2); // position
	shader.vertexAttribAdd(1, Renderer::AttribType::VEC2); // ray coordinate [-width/2, width/2], [-height / 2, height / 2]
	shader.vertexAttribsEnable();
	shader.uniformAdd("u_randTexture", Renderer::UniformType::INT);
	shader.uniformAdd("u_randSampler", Renderer::UniformType::VEC2);
	Renderer::Mat4<float> projection_matrix = Renderer::Math::projection2D(
		0.f, static_cast<float>(WINDOW_WIDTH),
		0.f, static_cast<float>(WINDOW_HEIGHT),
		1.f, -1.f
	);
	shader.setUniformInt("u_randTexture", 0);

	// create the shader to average each frame
	Renderer::Shader accumShader;
	accumShader.attach(&window);
	accumShader.createFromFile("res/average.vert", "res/average.frag", true);
	accumShader.vertexAttribAdd(0, Renderer::AttribType::VEC2);
	accumShader.vertexAttribAdd(1, Renderer::AttribType::VEC2);
	accumShader.vertexAttribsEnable();
	accumShader.uniformAdd("u_accumTexture", Renderer::UniformType::INT);
	accumShader.uniformAdd("u_frameCount", Renderer::UniformType::FLOAT);
	accumShader.setUniformInt("u_accumTexture", 0);

	// create the frame buffer for accumulation
	unsigned int accum_fbo;
	glGenFramebuffers(1, &accum_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, accum_fbo);

	GLuint accum_fbo_tex;
    glGenTextures(1, &accum_fbo_tex);
    glBindTexture(GL_TEXTURE_2D, accum_fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accum_fbo_tex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create the random normal vector texture
	unsigned char* rand_vec_data = new unsigned char[WINDOW_WIDTH * WINDOW_HEIGHT * 3];
	for(unsigned int i=0;i<WINDOW_WIDTH * WINDOW_HEIGHT;++i)
	{
		Renderer::Vec3<float> rand_vector;
		randVec3(rand_vector);
		rand_vector = (rand_vector + Renderer::Vec3<float>(1.f)) * Renderer::Vec3<float>(0.5f);

		// issue: does not include negatives
		rand_vec_data[i * 3] = static_cast<unsigned char>(rand_vector.x * 255);
		rand_vec_data[i * 3 + 1] = static_cast<unsigned char>(rand_vector.y * 255);
		rand_vec_data[i * 3 + 2] = static_cast<unsigned char>(rand_vector.z * 255);
	}

	float random_sampler[] = {0.f, 0.f};

	int frame_counter = 0;

	Renderer::Texture random_vectors(8);
	random_vectors.setTextureFilter(GL_NEAREST, GL_NEAREST);
	random_vectors.setTextureWrap(GL_REPEAT, GL_REPEAT);
	random_vectors.create(&window, WINDOW_WIDTH, WINDOW_HEIGHT, 3, rand_vec_data);
	delete[] rand_vec_data;

	auto previous_time = std::chrono::high_resolution_clock::now();

	unsigned int iteration = 0;

	auto unique_samples = new std::unordered_set<uint64_t>();

	while(window.isOpened())
	{
		renderer.bindTexture(&random_vectors);
		// calculate the fps
		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - previous_time);
		previous_time = current_time;
		float dt = elapsed_time.count() * 1e-9;
		//std::cout << 1.f / dt << "\n";

		random_sampler[0] = dis(gen);
		random_sampler[1] = dis(gen);
		frame_counter ++;
		accumShader.setUniformFloat("u_frameCount", frame_counter);

		// calculate the number of unique samples
		unsigned int displace_x = static_cast<uint32_t>(random_sampler[0] * WINDOW_WIDTH);
		unsigned int displace_y = static_cast<uint32_t>(random_sampler[1] * WINDOW_HEIGHT);
		uint64_t store_displace = displace_x;
		store_displace = store_displace << 32;
		store_displace |= displace_y;
		if(unique_samples->find(store_displace) == unique_samples->end())
			unique_samples->insert(store_displace);

		//std::cout << "unique random samples: " << unique_samples->size() << " / " << ++iteration << std::endl;

		// clear buffer
		glBindFramebuffer(GL_FRAMEBUFFER, accum_fbo);
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		//glClear(GL_COLOR_BUFFER_BIT);
		renderer.setBlendMode(Renderer::BlendMode::ADD);

		renderer.bindShader(&shader);
		random_vectors.bind(0);
		renderer.bindTexture(&random_vectors);

		shader.setUniformFloat("u_randSampler", random_sampler);
		renderer.beginShape(Renderer::DrawType::TRIANGLE, 4, 0);

		renderer.vertex2f(-1.f, 1.f);
		renderer.vertex2f(-WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

		renderer.nextVertex();
		
		renderer.vertex2f(1.f, 1.f);
		renderer.vertex2f( WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

		renderer.nextVertex();

		renderer.vertex2f(1.f, -1.f);
		renderer.vertex2f(WINDOW_WIDTH / 2.f, -WINDOW_HEIGHT / 2.f);

		renderer.nextVertex();

		renderer.vertex2f(-1.f, -1.f);
		renderer.vertex2f(-WINDOW_WIDTH / 2.f, -WINDOW_HEIGHT / 2.f);

		renderer.endShape();

		renderer.render();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WINDOW_WIDTH * 2.f, WINDOW_HEIGHT * 2.f);

		renderer.setBlendMode(Renderer::BlendMode::BLEND);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.bindShader(&accumShader);
		glBindTexture(GL_TEXTURE_2D, accum_fbo_tex);
		renderer.beginShape(Renderer::DrawType::TRIANGLE, 4, 0);
		renderer.vertex2f(-1.f, 1.f);
		renderer.vertex2f(0.f, 1.f);
		renderer.nextVertex();
		renderer.vertex2f(1.f, 1.f);
		renderer.vertex2f(1.f, 1.f);
		renderer.nextVertex();
		renderer.vertex2f(1.f, -1.f);
		renderer.vertex2f(1.f, 0.f);
		renderer.nextVertex();
		renderer.vertex2f(-1.f, -1.f);
		renderer.vertex2f(0.f, 0.f);
		renderer.endShape();
		
		renderer.render();

		window.swapBuffers();
		Renderer::Window::pollEvents();
	}

	delete unique_samples;

	glDeleteTextures(1, &accum_fbo_tex);
	glDeleteFramebuffers(1, &accum_fbo);

	return 0;
}

void randVec3(Renderer::Vec3<float>& _vec3)
{
	_vec3.x = dis(gen) * 2.f - 1.f;
	_vec3.y = dis(gen) * 2.f - 1.f;
	_vec3.z = dis(gen) * 2.f - 1.f;

	_vec3.normalize();
}
