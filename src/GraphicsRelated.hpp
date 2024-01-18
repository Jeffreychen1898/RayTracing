#pragma once
#define WINDOW_WIDTH 1024.f
#define WINDOW_HEIGHT 768.f

// fov of 90 deg: focal distance = window width
#define FOCAL_DISTANCE 1024.f

#define SAMPLES_PER_FRAME 10

#include <iostream>
#include <chrono>
#include <cmath>
#include <random>
#include <cfloat>
#include <unordered_set>

#include <renderer/Renderer.hpp>

struct GraphicsVariables
{
	Renderer::Window window;
	Renderer::Render renderer;
	Renderer::Shader rtShader;
	Renderer::Shader accumShader;

	unsigned int rtFbo;
	unsigned int rtFboTex;
	Renderer::Texture randVecTex;

	bool accumReset;

	GraphicsVariables()
		: randVecTex(8), accumReset(true)
	{ }
};

// static functions
static void graphicsVariablesInit();
static void graphicsWindowInit(GraphicsVariables* _vars);
static void graphicsRTShaderInit(GraphicsVariables* _vars);
static void graphicsAccumShaderInit(GraphicsVariables* _vars);
static void graphicsRTFBOInit(GraphicsVariables* _vars);
static void graphicsGenRandVecTex(GraphicsVariables* _vars);
static void graphicsRenderRT(GraphicsVariables* _vars);
static void graphicsRenderAccum(GraphicsVariables* _vars);

static void randVec3(Renderer::Vec3<float>& _vec3);

// static variables
static Renderer::Vec3<float> g_cameraPosition;
static Renderer::Vec3<float> g_cameraForward;
static Renderer::Vec3<float> g_cameraUp;

static bool g_cameraShouldMove[6]; // up down left right forward backward
static Renderer::Vec2<float> g_mousePos;
static Renderer::Vec2<float> g_prevMousePos;
static bool g_mousePressed;

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(0.f, 1.f);

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

GraphicsVariables* graphicsInit();

void graphicsUpdate(GraphicsVariables* _vars, float _dt);

void graphicsRender(GraphicsVariables* _vars);

void graphicsEnd(GraphicsVariables* _vars);
