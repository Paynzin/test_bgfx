#include "base.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>

#include "defines.h"

s32 SDL_main(s32 argc, c8** argv) {
	Allocator gpa = {
		.alloc = SDL_malloc,
		.realloc = SDL_realloc,
		.free = SDL_free
	};
	set_string_allocator(gpa);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return -1;
	}

	#define WIN_WIDTH 800
	#define WIN_HEIGHT 600
	
	SDL_Window* window = SDL_CreateWindow("test bgfx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) {
		return -1;
	}

	// get native window handle for windows
	SDL_SysWMinfo wm_info;
	SDL_VERSION(&wm_info.version);
	SDL_GetWindowWMInfo(window, &wm_info);
	
	bgfx::PlatformData bgfx_platform_data = {};
	bgfx_platform_data.nwh = wm_info.info.win.window;
	
	bgfx::Init bgfx_init = {};
	bgfx_init.type = bgfx::RendererType::Vulkan;
	bgfx_init.resolution.width = WIN_WIDTH;
	bgfx_init.resolution.height = WIN_HEIGHT;
	bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
	bgfx_init.platformData = bgfx_platform_data;
	
	bgfx::init(bgfx_init);
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR, 0xBCBCBCFF);
	bgfx::setViewRect(0, 0, 0, WIN_WIDTH, WIN_HEIGHT);
	
	b8 quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					quit = true;
				} break;
				
				case SDL_WINDOWEVENT: {
					SDL_WindowEvent window_event = event.window;
					if (window_event.event == SDL_WINDOWEVENT_RESIZED) {
						bgfx::reset(window_event.data1, window_event.data2, BGFX_RESET_VSYNC);
						bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);
					}
				} break;

				default: {
				} break;
			}
		}

		bgfx::touch(0);
		bgfx::setState(BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_B | BGFX_STATE_WRITE_A);
		
		static bgfx::VertexLayout triangle_vertex_input_layout = {};
		static bgfx::VertexBufferHandle triangle_vertex_buffer = {};
		static bgfx::IndexBufferHandle triangle_index_buffer = {};
		static bgfx::ProgramHandle triangle_program = {};
		static b8 is_initialized = false;
		if (!is_initialized) {
			File vertex_shader_bin = read_entire_file(create_string_from("shaders/spirv/default.vert.bin"), gpa);
			File fragment_shader_bin = read_entire_file(create_string_from("shaders/spirv/default.frag.bin"), gpa);
			BGFX_Vertex triangle_vertex_data[] {
				{ 0.0f, 0.5f }, // top
				{ -0.5f, -0.5f }, // left
				{ 0.5f, -0.5f }, // right
			};
			u16 triangle_index_data[] = { 0, 1, 2 };
			
			triangle_vertex_input_layout.begin()
				.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.end();
			
			triangle_vertex_buffer = bgfx::createVertexBuffer(bgfx::makeRef(triangle_vertex_data, sizeof(triangle_vertex_data)), triangle_vertex_input_layout);
			triangle_index_buffer = bgfx::createIndexBuffer(bgfx::makeRef(triangle_index_data, sizeof(triangle_index_data)));
			
			bgfx::ShaderHandle triangle_vertex_shader = bgfx::createShader(bgfx::copy(vertex_shader_bin.data, vertex_shader_bin.size));
			bgfx::ShaderHandle triangle_fragment_shader = bgfx::createShader(bgfx::copy(fragment_shader_bin.data, fragment_shader_bin.size));
			triangle_program = bgfx::createProgram(triangle_vertex_shader, triangle_fragment_shader, true);
			
			gpa.free(vertex_shader_bin.data);
			gpa.free(fragment_shader_bin.data);
			is_initialized = true;
		}
		
		bgfx::setVertexBuffer(0, triangle_vertex_buffer);
		bgfx::setIndexBuffer(triangle_index_buffer);
		
		bgfx::submit(0, triangle_program);
		
		bgfx::frame();
	}
	
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}