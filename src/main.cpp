#include "base.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>

#include "common.h"
#include "imgui.h"
#include "renderer.h"

#ifdef __linux__
#define SDL_main main
#endif

s32 SDL_main(s32 argc, c8** argv) {
	set_string_allocator(gpa);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return -1;
	}

	s32 win_width = 800;
	s32 win_height = 600;

	SDL_Window* window = SDL_CreateWindow("test bgfx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_width, win_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		return -1;
	}

	// get native window handle
	SDL_SysWMinfo wm_info;
	SDL_VERSION(&wm_info.version);
	SDL_GetWindowWMInfo(window, &wm_info);
	bgfx::PlatformData bgfx_platform_data = {};

	#ifdef _WIN32
	bgfx_platform_data.nwh = wm_info.info.win.window;
	#else
	// check for wayland support if not fallback to x11
	const c8* video_driver = SDL_GetCurrentVideoDriver();
	if (strcmp(video_driver, "wayland") == 0) {
		bgfx_platform_data.ndt = wm_info.info.wl.display;
		bgfx_platform_data.nwh = wm_info.info.wl.surface;
		bgfx_platform_data.type = bgfx::NativeWindowHandleType::Wayland;
	} else {
		bgfx_platform_data.ndt = wm_info.info.x11.display;
		bgfx_platform_data.nwh = (void*) wm_info.info.x11.window;
	}
	#endif

	bgfx::Init bgfx_init = {};
	bgfx_init.type = bgfx::RendererType::Vulkan;
	bgfx_init.resolution.width = win_width;
	bgfx_init.resolution.height = win_height;
	bgfx_init.resolution.reset = BGFX_RESET_NONE;
	bgfx_init.platformData = bgfx_platform_data;

	bgfx::init(bgfx_init);
	bgfx::setViewRect(0, 0, 0, win_width, win_height);

	imgui_init(window);

	b8 quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			imgui_process_events(&event);

			switch (event.type) {
				case SDL_QUIT: {
					quit = true;
				} break;

				case SDL_WINDOWEVENT: {
					SDL_WindowEvent window_event = event.window;
					if (window_event.event == SDL_WINDOWEVENT_RESIZED) {
						win_width = window_event.data1;
						win_height = window_event.data2;
						bgfx::reset(win_width, win_height, BGFX_RESET_NONE);
						bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);
					}
				} break;

				case SDL_KEYDOWN: {
					if (event.key.keysym.sym == SDLK_F11) {
						u32 window_flags = 0;
						if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)) {
	                        window_flags = SDL_WINDOW_FULLSCREEN;
	                    }
	                    SDL_SetWindowFullscreen(window, window_flags);
					}
				} break;

				default: {
				} break;
			}
		}
		
		static b8 is_renderer_initialized = false;
		if (!is_renderer_initialized) {
			renderer_initialize();
			is_renderer_initialized = true;
		}

		imgui_begin_frame();

		renderer_render(win_width, win_height);

		imgui_end_frame();
		bgfx::frame();
	}

	imgui_deinit();
	bgfx::shutdown();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}