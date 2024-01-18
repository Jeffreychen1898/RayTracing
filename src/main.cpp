#include <iostream>
#include <chrono>
#include <cmath>
#include <random>
#include <cfloat>
#include <unordered_set>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <renderer/Renderer.hpp>

#include "GraphicsRelated.hpp"
#include "UserInterface.hpp"

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

#define SAMPLES_PER_FRAME 10

#define PI 3.141592653589f

int main()
{
	GraphicsVariables* var_graphics = graphicsInit();
	Renderer::Window& window = var_graphics->window;
	Renderer::Render& renderer = var_graphics->renderer;
	Renderer::Shader& shader = var_graphics->rtShader;
	Renderer::Shader& accumShader = var_graphics->accumShader;

	unsigned int& accum_fbo = var_graphics->rtFbo;
	unsigned int& accum_fbo_tex = var_graphics->rtFboTex;
	Renderer::Texture& random_vectors = var_graphics->randVecTex;

	bool g_rtAccumReset = var_graphics->accumReset;

	auto previous_time = std::chrono::high_resolution_clock::now();

	UIVariables* var_ui = uiInit(var_graphics);

	while(window.isOpened())
	{
		// calculate the fps
		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - previous_time);
		previous_time = current_time;
		float dt = elapsed_time.count() * 1e-9;
		//std::cout << 1.f / dt << "\n";

		//update(dt);
		graphicsUpdate(var_graphics, dt);

		uiUpdate(var_ui, var_graphics, dt);
		graphicsRender(var_graphics);

		uiFrameEnd();

		window.swapBuffers();
		Renderer::Window::pollEvents();
	}

	uiEnd();

	graphicsEnd(var_graphics);

	return 0;
}
