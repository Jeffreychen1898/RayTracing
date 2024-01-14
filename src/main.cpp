#include <iostream>
#include <chrono>
#include <cmath>
#include <random>
#include <cfloat>
#include <unordered_set>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <renderer/Renderer.hpp>

// renderer todo:
//		renderer.clear()
//		renderer throw exception on too large of a shape
//		adjust point size
//		vector get angle
//		shader must be bound when resizing window?
//		vector and matrix *=, +=, etc
//		window.getWindow() return glfwWindow*

#define WINDOW_WIDTH 1024.f
#define WINDOW_HEIGHT 768.f

// fov of 90 deg: focal distance = window width
#define FOCAL_DISTANCE 1024.f

#define SAMPLES_PER_FRAME 5

#define PI 3.141592653589f

// camera position
static Renderer::Vec3<float> g_cameraPosition;
static Renderer::Vec3<float> g_cameraForward;
static Renderer::Vec3<float> g_cameraUp;
static bool g_rtAccumReset;

// controls
static bool g_cameraShouldMove[6]; // up down left right forward backward
static Renderer::Vec2<float> g_mousePos;
static Renderer::Vec2<float> g_prevMousePos;
static bool g_mousePressed;

// parameters
static float g_metallic;
static float g_roughness;
static Renderer::Vec3<float> g_skyColor;
static float g_skyIntensity;

// sampler
static Renderer::Vec2<float> g_randVecTexCoord;

class WindowEvents : public Renderer::WindowEvents
{
	public:

		Renderer::Shader* rt_shader;
		void KeyPressed(int _key, int _scancode, int _mods) override;
		void KeyReleased(int _key, int _scancode, int _mods) override;
		void MouseMove(double _x, double _y) override;
		void MousePressed(int _button, int _mods) override;
		void MouseReleased(int _button, int _mods) override;
};

static void randVec3(Renderer::Vec3<float>& _vec3);
static void renderPerPixel(int _x, int _y, float &_red, float &_green, float &_blue);
static void update(float _dt);
static void renderRT(Renderer::Render& _renderer);
static void renderAccum(Renderer::Render& _renderer);
static void initVariables();
static void initWindow(Renderer::Window& _window);
static void initRTShader(Renderer::Shader& _shader);
static void initAccumShader(Renderer::Shader& _shader);
static void createAccumFBO(unsigned int& _framebuffer, unsigned int& _texture);
static void getRandVecTexture(Renderer::Texture& _texture, Renderer::Window& _window);

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.f, 1.f);

int main()
{
	initVariables();
	Renderer::Window::GLFWInit();

	// create the window
	Renderer::Window window;
	initWindow(window);

	// initialize imgui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");

	// create the renderer
	Renderer::Render renderer;
	renderer.attach(&window);
	renderer.init();

	// enable multiple frame buffers
	GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, draw_buffers);

	// create the rt shader
	Renderer::Shader shader;
	shader.attach(&window);
	initRTShader(shader);

	// create the shader to average each frame
	Renderer::Shader accumShader;
	accumShader.attach(&window);
	initAccumShader(accumShader);

	// create the frame buffer for accumulation
	unsigned int accum_fbo;
	unsigned int accum_fbo_tex;
	createAccumFBO(accum_fbo, accum_fbo_tex);

	// create the random normal vector texture
	Renderer::Texture random_vectors(8);
	getRandVecTexture(random_vectors, window);

	auto previous_time = std::chrono::high_resolution_clock::now();

	unsigned int iteration = 0;

	while(window.isOpened())
	{
		// calculate the fps
		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - previous_time);
		previous_time = current_time;
		float dt = elapsed_time.count() * 1e-9;
		//std::cout << 1.f / dt << "\n";

		update(dt);

		// imgui rendering

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


		// render pass: ray tracing
		glBindFramebuffer(GL_FRAMEBUFFER, accum_fbo);
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		glBlendFunc(GL_ONE, GL_ONE);

		random_vectors.bind(0);
		renderer.bindShader(&shader);

		// imgui shit
        // Create a window
        ImGui::Begin("Controls");

		// imgui controls
		if(ImGui::SliderFloat("Roughness", &g_roughness, 0.01f, 1.0f))
		{
			g_rtAccumReset = true;
			shader.setUniformFloat("u_roughness", g_roughness);
		}
		if(ImGui::SliderFloat("Metallic", &g_metallic, 0.01f, 1.0f))
		{
			g_rtAccumReset = true;
			shader.setUniformFloat("u_metallic", g_metallic);
		}

		//_shader.setUniformFloat("u_skyColor", *g_skyColor);
		if(ImGui::DragFloat("Sky Intensity", &g_skyIntensity, 0.01f, 0.f, 100.f, "%.2f", ImGuiSliderFlags_Logarithmic))
		{
			g_rtAccumReset = true;
			shader.setUniformFloat("u_skyIntensity", g_skyIntensity);
		}

        ImGui::End();
		// end of imgui shit

		if(g_rtAccumReset)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			shader.setUniformFloat("u_cameraPosition", *g_cameraPosition);
			shader.setUniformFloat("u_cameraForward", *g_cameraForward);
			shader.setUniformFloat("u_cameraUp", *g_cameraUp);

			g_rtAccumReset = false;
		}

		for(int32_t i=0;i<SAMPLES_PER_FRAME;++i)
		{
			// sample tex coord for the texture
			g_randVecTexCoord.x = dis(gen);
			g_randVecTexCoord.y = dis(gen);
			shader.setUniformFloat("u_randSampler", *g_randVecTexCoord);

			renderRT(renderer);
			renderer.render();
		}

		// render pass: accumulation (average the colors)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WINDOW_WIDTH * 2.f, WINDOW_HEIGHT * 2.f);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer.setBlendMode(Renderer::BlendMode::BLEND);

		renderer.bindShader(&accumShader);
		glBindTexture(GL_TEXTURE_2D, accum_fbo_tex);

		renderAccum(renderer);
		renderer.render();

		// imgui rendering
		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
		Renderer::Window::pollEvents();
	}
	// imgui shut down
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glDeleteTextures(1, &accum_fbo_tex);
	glDeleteFramebuffers(1, &accum_fbo);

	return 0;
}

void initVariables()
{
	g_rtAccumReset = true;

	g_cameraPosition.x = 0.f;
	g_cameraPosition.y = 0.f;
	g_cameraPosition.z = 0.f;

	g_cameraForward.x = 0.f;
	g_cameraForward.y = 0.f;
	g_cameraForward.z = -1.f;

	g_cameraUp.x = 0.f;
	g_cameraUp.y = 1.f;
	g_cameraUp.z = 0.f;

	g_randVecTexCoord.x = 0.f;
	g_randVecTexCoord.y = 0.f;

	g_metallic = 0.5f;
	g_roughness = 0.5f;

	g_skyColor.x = 0.3f;
	g_skyColor.y = 0.3f;
	g_skyColor.z = 0.3f;
	g_skyIntensity = 1.f;
}

void initRTShader(Renderer::Shader& _shader)
{
	_shader.createFromFile("res/basic.vert", "res/basic.frag", true);
	_shader.vertexAttribAdd(0, Renderer::AttribType::VEC2); // position
	_shader.vertexAttribAdd(1, Renderer::AttribType::VEC2); // ray coordinate [-width/2, width/2], [-height / 2, height / 2]
	_shader.vertexAttribsEnable();
	_shader.uniformAdd("u_randTexture", Renderer::UniformType::INT);
	_shader.uniformAdd("u_randSampler", Renderer::UniformType::VEC2);
	_shader.uniformAdd("u_cameraPosition", Renderer::UniformType::VEC3);
	_shader.uniformAdd("u_cameraForward", Renderer::UniformType::VEC3);
	_shader.uniformAdd("u_cameraUp", Renderer::UniformType::VEC3);
	_shader.uniformAdd("u_focalDistance", Renderer::UniformType::FLOAT);
	_shader.uniformAdd("u_metallic", Renderer::UniformType::FLOAT);
	_shader.uniformAdd("u_roughness", Renderer::UniformType::FLOAT);
	_shader.uniformAdd("u_skyColor", Renderer::UniformType::VEC3);
	_shader.uniformAdd("u_skyIntensity", Renderer::UniformType::FLOAT);
	_shader.setUniformInt("u_randTexture", 0);
	_shader.setUniformFloat("u_cameraPosition", *g_cameraPosition);
	_shader.setUniformFloat("u_cameraForward", *g_cameraForward);
	_shader.setUniformFloat("u_cameraUp", *g_cameraUp);
	_shader.setUniformFloat("u_focalDistance", FOCAL_DISTANCE);
	_shader.setUniformFloat("u_metallic", g_metallic);
	_shader.setUniformFloat("u_roughness", g_roughness);
	_shader.setUniformFloat("u_skyColor", *g_skyColor);
	_shader.setUniformFloat("u_skyIntensity", g_skyIntensity);
}

void initAccumShader(Renderer::Shader& _shader)
{
	_shader.createFromFile("res/average.vert", "res/average.frag", true);
	_shader.vertexAttribAdd(0, Renderer::AttribType::VEC2);
	_shader.vertexAttribAdd(1, Renderer::AttribType::VEC2);
	_shader.vertexAttribsEnable();
	_shader.uniformAdd("u_accumTexture", Renderer::UniformType::INT);
	_shader.setUniformInt("u_accumTexture", 0);
}

void createAccumFBO(unsigned int& _framebuffer, unsigned int& _texture)
{
	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void getRandVecTexture(Renderer::Texture& _texture, Renderer::Window& _window)
{
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

	_texture.setTextureFilter(GL_NEAREST, GL_NEAREST);
	_texture.setTextureWrap(GL_REPEAT, GL_REPEAT);
	_texture.create(&_window, WINDOW_WIDTH, WINDOW_HEIGHT, 3, rand_vec_data);
	delete[] rand_vec_data;

}

void initWindow(Renderer::Window& _window)
{
	WindowEvents* win_evt = new WindowEvents;

	_window.init(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Tracing");
	_window.addEvents(win_evt);
	//_window.setVSync(false);
}

void renderRT(Renderer::Render& _renderer)
{
	_renderer.beginShape(Renderer::DrawType::TRIANGLE, 4, 0);

	_renderer.vertex2f(-1.f, 1.f);
	_renderer.vertex2f(-WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

	_renderer.nextVertex();
	
	_renderer.vertex2f(1.f, 1.f);
	_renderer.vertex2f( WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

	_renderer.nextVertex();

	_renderer.vertex2f(1.f, -1.f);
	_renderer.vertex2f(WINDOW_WIDTH / 2.f, -WINDOW_HEIGHT / 2.f);

	_renderer.nextVertex();

	_renderer.vertex2f(-1.f, -1.f);
	_renderer.vertex2f(-WINDOW_WIDTH / 2.f, -WINDOW_HEIGHT / 2.f);

	_renderer.endShape();
}

void renderAccum(Renderer::Render& _renderer)
{
	_renderer.beginShape(Renderer::DrawType::TRIANGLE, 4, 0);

	_renderer.vertex2f(-1.f, 1.f);
	_renderer.vertex2f(0.f, 1.f);

	_renderer.nextVertex();

	_renderer.vertex2f(1.f, 1.f);
	_renderer.vertex2f(1.f, 1.f);

	_renderer.nextVertex();

	_renderer.vertex2f(1.f, -1.f);
	_renderer.vertex2f(1.f, 0.f);

	_renderer.nextVertex();

	_renderer.vertex2f(-1.f, -1.f);
	_renderer.vertex2f(0.f, 0.f);

	_renderer.endShape();
}

void update(float _dt)
{
	// code to move camera position
	
	Renderer::Vec3<float> camera_right = g_cameraForward.cross(g_cameraUp);
	Renderer::Vec3<float> camera_forward = g_cameraUp.cross(camera_right);
	if(g_cameraShouldMove[0]) // move up
	{
		g_cameraPosition = g_cameraPosition + g_cameraUp * (2.f * _dt);
		g_rtAccumReset = true;
	}
	if(g_cameraShouldMove[1]) // move down
	{
		g_cameraPosition = g_cameraPosition - g_cameraUp * (2.f * _dt);
		g_rtAccumReset = true;
	}
	if(g_cameraShouldMove[2]) // move left
	{
		g_cameraPosition = g_cameraPosition - camera_right * (2.f * _dt);
		g_rtAccumReset = true;
	}
	if(g_cameraShouldMove[3]) // move right
	{
		g_cameraPosition = g_cameraPosition + camera_right * (2.f * _dt);
		g_rtAccumReset = true;
	}
	if(g_cameraShouldMove[4]) // move forward
	{
		g_cameraPosition = g_cameraPosition + camera_forward * (2.f * _dt);
		g_rtAccumReset = true;
	}
	if(g_cameraShouldMove[5]) // move backward
	{
		g_cameraPosition = g_cameraPosition - camera_forward * (2.f * _dt);
		g_rtAccumReset = true;
	}

	if(g_mousePressed && g_prevMousePos != g_mousePos) // mouse drag for rotation
	{
		float one_over_focal_dist = 1.f / FOCAL_DISTANCE;
		Renderer::Vec2<float> diff = (g_mousePos - g_prevMousePos) * one_over_focal_dist;

		g_cameraForward = g_cameraForward + camera_right * -diff.x + g_cameraUp * diff.y;
		g_cameraForward.normalize();

		g_rtAccumReset = true;
	}

	g_prevMousePos = g_mousePos;
}

void randVec3(Renderer::Vec3<float>& _vec3)
{
	float theta = dis(gen) * 3.14159265f * 2.f;
	float phi = std::acos(2.f * dis(gen) - 1.f);
	_vec3.x = std::sin(phi) * std::cos(theta);
	_vec3.y = std::sin(phi) * std::sin(theta);
	_vec3.z = std::cos(phi);

	_vec3.normalize();
}

void WindowEvents::KeyPressed(int _key, int _scancode, int _mods)
{
	switch(_key)
	{
		case 87: // W
			g_cameraShouldMove[4] = true;
			break;
		case 83: // S
			g_cameraShouldMove[5] = true;
			break;
		case 65: // A
			g_cameraShouldMove[2] = true;
			break;
		case 68: // D
			g_cameraShouldMove[3] = true;
			break;
		case 81: // Q
			g_cameraShouldMove[1] = true;
			break;
		case 69: // E
			g_cameraShouldMove[0] = true;
			break;
	}
}

void WindowEvents::KeyReleased(int _key, int _scancode, int _mods)
{
	switch(_key)
	{
		case 87: // W
			g_cameraShouldMove[4] = false;
			break;
		case 83: // S
			g_cameraShouldMove[5] = false;
			break;
		case 65: // A
			g_cameraShouldMove[2] = false;
			break;
		case 68: // D
			g_cameraShouldMove[3] = false;
			break;
		case 81: // Q
			g_cameraShouldMove[1] = false;
			break;
		case 69: // E
			g_cameraShouldMove[0] = false;
			break;
	}
}

void WindowEvents::MouseMove(double _x, double _y)
{
	g_mousePos.x = _x;
	g_mousePos.y = _y;
}

void WindowEvents::MousePressed(int _button, int _mods)
{
	if(_button == GLFW_MOUSE_BUTTON_RIGHT)
		g_mousePressed = true;

	if(_mods != GLFW_MOD_SHIFT)
		return;

	if(_button == GLFW_MOUSE_BUTTON_LEFT)
		g_mousePressed = true;
}

void WindowEvents::MouseReleased(int _button, int _mods)
{
	if(_button == GLFW_MOUSE_BUTTON_LEFT)
		g_mousePressed = false;
	if(_button == GLFW_MOUSE_BUTTON_RIGHT)
		g_mousePressed = false;
}
