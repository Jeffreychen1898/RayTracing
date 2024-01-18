#include "GraphicsRelated.hpp"

GraphicsVariables* graphicsInit()
{
	graphicsVariablesInit();

	GraphicsVariables* graphics_variables = new GraphicsVariables;

	graphicsWindowInit(graphics_variables);

	graphics_variables->renderer.attach(&graphics_variables->window);
	graphics_variables->renderer.init();

	GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, draw_buffers);

	graphics_variables->rtShader.attach(&graphics_variables->window);
	graphicsRTShaderInit(graphics_variables);

	graphics_variables->accumShader.attach(&graphics_variables->window);
	graphicsAccumShaderInit(graphics_variables);

	graphicsRTFBOInit(graphics_variables);

	graphicsGenRandVecTex(graphics_variables);

	return graphics_variables;
}

void graphicsUpdate(GraphicsVariables* _vars, float _dt)
{
	Renderer::Vec3<float> camera_right = g_cameraForward.cross(g_cameraUp);
	Renderer::Vec3<float> camera_forward = g_cameraUp.cross(camera_right);
	if(g_cameraShouldMove[0]) // move up
	{
		g_cameraPosition = g_cameraPosition + g_cameraUp * (2.f * _dt);
		_vars->accumReset = true;
	}
	if(g_cameraShouldMove[1]) // move down
	{
		g_cameraPosition = g_cameraPosition - g_cameraUp * (2.f * _dt);
		_vars->accumReset = true;
	}
	if(g_cameraShouldMove[2]) // move left
	{
		g_cameraPosition = g_cameraPosition - camera_right * (2.f * _dt);
		_vars->accumReset = true;
	}
	if(g_cameraShouldMove[3]) // move right
	{
		g_cameraPosition = g_cameraPosition + camera_right * (2.f * _dt);
		_vars->accumReset = true;
	}
	if(g_cameraShouldMove[4]) // move forward
	{
		g_cameraPosition = g_cameraPosition + camera_forward * (2.f * _dt);
		_vars->accumReset = true;
	}
	if(g_cameraShouldMove[5]) // move backward
	{
		g_cameraPosition = g_cameraPosition - camera_forward * (2.f * _dt);
		_vars->accumReset = true;
	}

	if(g_mousePressed && g_prevMousePos != g_mousePos) // mouse drag for rotation
	{
		float one_over_focal_dist = 1.f / FOCAL_DISTANCE;
		Renderer::Vec2<float> diff = (g_mousePos - g_prevMousePos) * one_over_focal_dist;

		g_cameraForward = g_cameraForward + camera_right * -diff.x + g_cameraUp * diff.y;
		g_cameraForward.normalize();

		_vars->accumReset = true;
	}

	g_prevMousePos = g_mousePos;
}

void graphicsRender(GraphicsVariables* _vars)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _vars->rtFbo);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glBlendFunc(GL_ONE, GL_ONE);

	_vars->randVecTex.bind(0);
	_vars->renderer.bindShader(&_vars->rtShader);

	if(_vars->accumReset)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		_vars->rtShader.setUniformFloat("u_cameraPosition", *g_cameraPosition);
		_vars->rtShader.setUniformFloat("u_cameraForward", *g_cameraForward);
		_vars->rtShader.setUniformFloat("u_cameraUp", *g_cameraUp);

		_vars->accumReset = false;
	}

	for(int32_t i=0;i<SAMPLES_PER_FRAME;++i)
	{
		// sample tex coord for the texture
		g_randVecTexCoord.x = dis(gen);
		g_randVecTexCoord.y = dis(gen);
		_vars->rtShader.setUniformFloat("u_randSampler", *g_randVecTexCoord);

		graphicsRenderRT(_vars);
		_vars->renderer.render();
	}

	// render pass: accumulation (average the colors)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH * 2.f, WINDOW_HEIGHT * 2.f);
	glClear(GL_COLOR_BUFFER_BIT);

	_vars->renderer.setBlendMode(Renderer::BlendMode::BLEND);

	_vars->renderer.bindShader(&_vars->accumShader);
	glBindTexture(GL_TEXTURE_2D, _vars->rtFboTex);

	graphicsRenderAccum(_vars);
	_vars->renderer.render();
}

void graphicsEnd(GraphicsVariables* _vars)
{
	glDeleteTextures(1, &_vars->rtFboTex);
	glDeleteFramebuffers(1, &_vars->rtFbo);
}

// rest of these are static functions
void graphicsVariablesInit()
{
	g_cameraPosition.x = 0.f;
	g_cameraPosition.y = 0.f;
	g_cameraPosition.z = 0.f;

	g_cameraForward.x = 0.f;
	g_cameraForward.y = 0.f;
	g_cameraForward.z = -1.f;

	g_cameraUp.x = 0.f;
	g_cameraUp.y = 1.f;
	g_cameraUp.z = 0.f;

	for(int i=0;i<6;++i)
		g_cameraShouldMove[i] = false;

	g_mousePos.x = 0.f;
	g_mousePos.y = 0.f;

	g_prevMousePos.x = 0.f;
	g_prevMousePos.y = 0.f;
	g_mousePressed = false;

	g_randVecTexCoord.x = 0.f;
	g_randVecTexCoord.y = 0.f;
}

void graphicsWindowInit(GraphicsVariables* _vars)
{
	Renderer::Window::GLFWInit();
	WindowEvents* window_events = new WindowEvents;

	_vars->window.init(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Tracing");
	_vars->window.addEvents(window_events);
	_vars->window.setVSync(true);
}

void graphicsRTShaderInit(GraphicsVariables* _vars)
{
	_vars->rtShader.createFromFile("res/basic.vert", "res/basic.frag", true);
	_vars->rtShader.vertexAttribAdd(0, Renderer::AttribType::VEC2); // position
	_vars->rtShader.vertexAttribAdd(1, Renderer::AttribType::VEC2); // ray coordinate [-width/2, width/2], [-height / 2, height / 2]
	_vars->rtShader.vertexAttribsEnable();
	_vars->rtShader.uniformAdd("u_randTexture", Renderer::UniformType::INT);
	_vars->rtShader.uniformAdd("u_randSampler", Renderer::UniformType::VEC2);
	_vars->rtShader.uniformAdd("u_cameraPosition", Renderer::UniformType::VEC3);
	_vars->rtShader.uniformAdd("u_cameraForward", Renderer::UniformType::VEC3);
	_vars->rtShader.uniformAdd("u_cameraUp", Renderer::UniformType::VEC3);
	_vars->rtShader.uniformAdd("u_focalDistance", Renderer::UniformType::FLOAT);
	_vars->rtShader.uniformAdd("u_metallic", Renderer::UniformType::FLOAT);
	_vars->rtShader.uniformAdd("u_roughness", Renderer::UniformType::FLOAT);
	_vars->rtShader.uniformAdd("u_skyColor", Renderer::UniformType::VEC3);
	_vars->rtShader.uniformAdd("u_skyIntensity", Renderer::UniformType::FLOAT);
	_vars->rtShader.setUniformInt("u_randTexture", 0);
	_vars->rtShader.setUniformFloat("u_cameraPosition", *g_cameraPosition);
	_vars->rtShader.setUniformFloat("u_cameraForward", *g_cameraForward);
	_vars->rtShader.setUniformFloat("u_cameraUp", *g_cameraUp);
	_vars->rtShader.setUniformFloat("u_focalDistance", FOCAL_DISTANCE);

	float sky_color[] = {0.5f, 0.5f, 0.5f};

	_vars->rtShader.setUniformFloat("u_metallic", 1.f);
	_vars->rtShader.setUniformFloat("u_roughness", 0.3f);
	_vars->rtShader.setUniformFloat("u_skyColor", sky_color);
	_vars->rtShader.setUniformFloat("u_skyIntensity", 1.f);
}

void graphicsAccumShaderInit(GraphicsVariables* _vars)
{
	_vars->accumShader.createFromFile("res/average.vert", "res/average.frag", true);
	_vars->accumShader.vertexAttribAdd(0, Renderer::AttribType::VEC2);
	_vars->accumShader.vertexAttribAdd(1, Renderer::AttribType::VEC2);
	_vars->accumShader.vertexAttribsEnable();
	_vars->accumShader.uniformAdd("u_accumTexture", Renderer::UniformType::INT);
	_vars->accumShader.setUniformInt("u_accumTexture", 0);
}

void graphicsRTFBOInit(GraphicsVariables* _vars)
{
	glGenFramebuffers(1, &_vars->rtFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _vars->rtFbo);

    glGenTextures(1, &_vars->rtFboTex);
    glBindTexture(GL_TEXTURE_2D, _vars->rtFboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _vars->rtFboTex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void graphicsGenRandVecTex(GraphicsVariables* _vars)
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

	_vars->randVecTex.setTextureFilter(GL_NEAREST, GL_NEAREST);
	_vars->randVecTex.setTextureWrap(GL_REPEAT, GL_REPEAT);
	_vars->randVecTex.create(&_vars->window, WINDOW_WIDTH, WINDOW_HEIGHT, 3, rand_vec_data);
	delete[] rand_vec_data;
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

void graphicsRenderRT(GraphicsVariables* _vars)
{
	_vars->renderer.beginShape(Renderer::DrawType::TRIANGLE, 4, 0);

	_vars->renderer.vertex2f(-1.f, 1.f);
	_vars->renderer.vertex2f(-WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

	_vars->renderer.nextVertex();
	
	_vars->renderer.vertex2f(1.f, 1.f);
	_vars->renderer.vertex2f( WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

	_vars->renderer.nextVertex();

	_vars->renderer.vertex2f(1.f, -1.f);
	_vars->renderer.vertex2f(WINDOW_WIDTH / 2.f, -WINDOW_HEIGHT / 2.f);

	_vars->renderer.nextVertex();

	_vars->renderer.vertex2f(-1.f, -1.f);
	_vars->renderer.vertex2f(-WINDOW_WIDTH / 2.f, -WINDOW_HEIGHT / 2.f);

	_vars->renderer.endShape();
}

void graphicsRenderAccum(GraphicsVariables* _vars)
{
	_vars->renderer.beginShape(Renderer::DrawType::TRIANGLE, 4, 0);

	_vars->renderer.vertex2f(-1.f, 1.f);
	_vars->renderer.vertex2f(0.f, 1.f);

	_vars->renderer.nextVertex();

	_vars->renderer.vertex2f(1.f, 1.f);
	_vars->renderer.vertex2f(1.f, 1.f);

	_vars->renderer.nextVertex();

	_vars->renderer.vertex2f(1.f, -1.f);
	_vars->renderer.vertex2f(1.f, 0.f);

	_vars->renderer.nextVertex();

	_vars->renderer.vertex2f(-1.f, -1.f);
	_vars->renderer.vertex2f(0.f, 0.f);

	_vars->renderer.endShape();
}

// window events
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
