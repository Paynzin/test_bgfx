#include "base.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include "imgui_bgfx/imgui_impl_bgfx.h"
#include <SDL.h>

void imgui_init(SDL_Window* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	
	ImGui::StyleColorsDark();
	
	ImGui_ImplSDL2_InitForOther(window);
	ImGui_ImplBgfx_Init(0);
}

void imgui_deinit() {
	ImGui_ImplBgfx_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void imgui_begin_frame() {
	ImGui_ImplBgfx_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void imgui_end_frame() {
	ImGui::Render();
	ImGui_ImplBgfx_RenderDrawData(ImGui::GetDrawData());
}

void imgui_process_events(SDL_Event* event) {
	ImGui_ImplSDL2_ProcessEvent(event);
}