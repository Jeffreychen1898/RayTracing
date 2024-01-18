#include "UserInterface.hpp"

UIVariables* uiInit(GraphicsVariables* _graphicsVars)
{
	UIVariables* ui_variables = new UIVariables;
	ui_variables->roughness = 0.5f;
	ui_variables->metallic = 0.5f;
	ui_variables->skyIntensity = 3.f;

	// initialize imgui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(_graphicsVars->window.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");

	return ui_variables;
}

void uiUpdate(UIVariables* _uiVars, GraphicsVariables* _graphicsVars, float _dt)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	_graphicsVars->rtShader.bind();

	ImGui::Begin("Controls");
	ImGui::Text("FPS: %f", 1.f / _dt);

	// imgui controls
	ImGui::Text("Roughness: ");
	if(ImGui::SliderFloat("Roughness", &_uiVars->roughness, 0.01f, 1.0f))
	{
		_graphicsVars->accumReset = true;
		_graphicsVars->rtShader.setUniformFloat("u_roughness", _uiVars->roughness);
	}
	ImGui::Text("Metallic: ");
	if(ImGui::SliderFloat("Metallic", &_uiVars->metallic, 0.01f, 1.0f))
	{
		_graphicsVars->accumReset = true;
		_graphicsVars->rtShader.setUniformFloat("u_metallic", _uiVars->metallic);
	}
	ImGui::Text("Sky Intensity: ");
	//_shader.setUniformFloat("u_skyColor", *g_skyColor);
	if(ImGui::DragFloat("Sky Intensity", &_uiVars->skyIntensity, 0.1f, 0.f, 100.f, "%.2f", ImGuiSliderFlags_Logarithmic))
	{
		_graphicsVars->accumReset = true;
		_graphicsVars->rtShader.setUniformFloat("u_skyIntensity", _uiVars->skyIntensity);
	}

	ImGui::End();
}

void uiFrameEnd()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void uiEnd()
{
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
