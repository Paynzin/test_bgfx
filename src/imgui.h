#pragma once

#include "base.h"
#include <SDL.h>

void imgui_init(SDL_Window* window);
void imgui_deinit();
void imgui_begin_frame();
void imgui_end_frame();
void imgui_process_events(SDL_Event* event);