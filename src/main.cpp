#include "base.h"
#include <SDL.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <imgui.h>

#include "defines.h"
#include "imgui.h"
#include "textures.h"

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
	bgfx::setViewRect(0, 0, 0, WIN_WIDTH, WIN_HEIGHT);
	
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
						bgfx::reset(window_event.data1, window_event.data2, BGFX_RESET_VSYNC);
						bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);
					}
				} break;

				default: {
				} break;
			}
		}
		
		imgui_begin_frame();
		
		static BGFX_Color clear_color = { 0.0f, 0.0f, 0.75f, 1.00f };
		static BGFX_Color quad_color = { 1.0f, 1.0f, 1.0f, 1.00f };
		
		ImGui::Begin("cool overlay");
		ImGui::ColorEdit3("clear color", (f32*) &clear_color);
		ImGui::ColorEdit3("quad color", (f32*) &quad_color);
		ImGui::End();

		bgfx::touch(0);
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR, bgfx_color(clear_color));
		
		static bgfx::VertexLayout quad_vertex_input_layout = {};
		static bgfx::VertexBufferHandle quad_vertex_buffer = {};
		static bgfx::IndexBufferHandle quad_index_buffer = {};
		static bgfx::ProgramHandle quad_program = {};
		static bgfx::TextureHandle quad_texture = {};
		static bgfx::UniformHandle quad_texture_uniform = {};
		static bgfx::UniformHandle quad_color_uniform = {};
		static b8 is_initialized = false;
		if (!is_initialized) {
			BGFX_Texture texture = bgfx_load_texture(create_string_from("assets/texture.ktx"), gpa);
			File vertex_shader_bin = read_entire_file(create_string_from("shaders/spirv/default.vert.bin"), gpa);
			File fragment_shader_bin = read_entire_file(create_string_from("shaders/spirv/default.frag.bin"), gpa);
			BGFX_Vertex quad_vertex_data[] {
				// top left
				{ { -0.5f, 0.5f }, { 0.0f, 0.0f } },
				// top right
				{ { 0.5f, 0.5f }, { 1.0f, 0.0f } },
				// bottom left
				{ { -0.5f, -0.5f }, { 0.0f, 1.0f } },
				// bottom right
				{ { 0.5f, -0.5f }, { 1.0f, 1.0f } },
			};
			u16 quad_index_data[] = { 0, 1, 2, /**/ 1, 3, 2, };
			
			quad_vertex_input_layout.begin()
				.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true)
			.end();
			
			quad_vertex_buffer = bgfx::createVertexBuffer(bgfx::makeRef(quad_vertex_data, sizeof(quad_vertex_data)), quad_vertex_input_layout);
			quad_index_buffer = bgfx::createIndexBuffer(bgfx::makeRef(quad_index_data, sizeof(quad_index_data)));
			
			u64 texture_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_UVW_CLAMP | BGFX_SAMPLER_POINT;
			quad_texture = bgfx::createTexture2D(texture.width, texture.height, texture.has_mips, texture.layers, texture.format, texture_flags, texture.data);
			
			bgfx::ShaderHandle quad_vertex_shader = bgfx::createShader(bgfx::copy(vertex_shader_bin.data, vertex_shader_bin.size));
			bgfx::ShaderHandle quad_fragment_shader = bgfx::createShader(bgfx::copy(fragment_shader_bin.data, fragment_shader_bin.size));
			
			quad_color_uniform = bgfx::createUniform("u_quad_color", bgfx::UniformType::Vec4);
			quad_texture_uniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
			
			quad_program = bgfx::createProgram(quad_vertex_shader, quad_fragment_shader, true);

			gpa.free(vertex_shader_bin.data);
			gpa.free(fragment_shader_bin.data);
			is_initialized = true;
		}
		
		bgfx::setVertexBuffer(0, quad_vertex_buffer);
		bgfx::setIndexBuffer(quad_index_buffer);
		bgfx::setTexture(0, quad_texture_uniform, quad_texture);
		bgfx::setUniform(quad_color_uniform, &quad_color);
		
		bgfx::submit(0, quad_program);
		
		imgui_end_frame();
		bgfx::frame();
	}
	
	imgui_deinit();
	bgfx::shutdown();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}