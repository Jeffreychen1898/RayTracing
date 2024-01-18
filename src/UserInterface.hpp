#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "GraphicsRelated.hpp"

struct UIVariables
{
	float roughness;
	float metallic;
	float skyIntensity;
};

UIVariables* uiInit(GraphicsVariables* _graphicsVars);
void uiUpdate(UIVariables* _uiVars, GraphicsVariables* _graphicsVars, float _dt);
void uiFrameEnd();
void uiEnd();
