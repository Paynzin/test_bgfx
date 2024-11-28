#include "base.h"
#include <SDL.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "renderer.h"
#include "common.h"
#include "textures.h"

static Renderer_Runtime renderer_runtime = {};

void renderer_initialize() {
	// query host performance frequency
	renderer_runtime.performance_frequency = SDL_GetPerformanceFrequency();
	
	// initialize some runtime values
	{
		renderer_runtime.clear_color = { 0.0f, 0.0f, 0.75f, 1.00f };
		renderer_runtime.quad_color = { 1.0f, 1.0f, 1.0f, 1.00f };
		renderer_runtime.quad_scale = 1.0f;
	}
	
	// create and initialize some gpu data
	{
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
		
		bgfx::VertexLayout quad_vertex_input_layout = {};
		quad_vertex_input_layout.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true)
		.end();
		
		renderer_runtime.quad_vertex_buffer = bgfx::createVertexBuffer(bgfx::copy(quad_vertex_data, sizeof(quad_vertex_data)), quad_vertex_input_layout);
		renderer_runtime.quad_index_buffer = bgfx::createIndexBuffer(bgfx::copy(quad_index_data, sizeof(quad_index_data)));
		
		u64 texture_flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_UVW_CLAMP | BGFX_SAMPLER_POINT;
		renderer_runtime.quad_texture = bgfx::createTexture2D(texture.width, texture.height, texture.has_mips, texture.layers, texture.format, texture_flags, texture.data);
		
		bgfx::ShaderHandle quad_vertex_shader = bgfx::createShader(bgfx::copy(vertex_shader_bin.data, vertex_shader_bin.size));
		bgfx::ShaderHandle quad_fragment_shader = bgfx::createShader(bgfx::copy(fragment_shader_bin.data, fragment_shader_bin.size));
		renderer_runtime.quad_program = bgfx::createProgram(quad_vertex_shader, quad_fragment_shader, true);
		gpa.free(vertex_shader_bin.data);
		gpa.free(fragment_shader_bin.data);
	}
	
	// create shader uniforms
	{	
		renderer_runtime.quad_transform_uniform = bgfx::createUniform("u_quad_transform", bgfx::UniformType::Mat4);
		renderer_runtime.quad_color_uniform = bgfx::createUniform("u_quad_color", bgfx::UniformType::Vec4);
		renderer_runtime.quad_texture_uniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
	}
}

void renderer_render(s32 win_width, s32 win_height) {
	// calc delta && fps
	{
	    static u64 last_time;
	    u64 current_time = SDL_GetPerformanceCounter();
	    renderer_runtime.delta = (f32) (current_time - last_time) / renderer_runtime.performance_frequency;
	    renderer_runtime.fps = 1.0f / renderer_runtime.delta;
	    last_time = current_time;
	}
    
    // ui
    {
    	ImGui::Begin("cool overlay");
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "FPS: %f", renderer_runtime.fps);
		ImGui::ColorEdit3("clear color", (f32*) &renderer_runtime.clear_color);
		ImGui::ColorEdit3("quad color", (f32*) &renderer_runtime.quad_color);
		ImGui::SliderFloat("quad scale", &renderer_runtime.quad_scale, 0.0f, 3.0f);
		ImGui::End();
    }
    
    // prepare for rendering
    {
    	bgfx::touch(0);
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR, bgfx_color(renderer_runtime.clear_color));
    }
    
    // render quad
    {
    	// calc quad transform matrix
    	f32 scale = renderer_runtime.quad_scale;
    	glm::mat4 quad_transform = glm::mat4(1.0f);
		quad_transform = glm::scale(quad_transform, glm::vec3(scale, scale, scale));
    	
    	// bind resources
    	bgfx::setVertexBuffer(0, renderer_runtime.quad_vertex_buffer);
		bgfx::setIndexBuffer(renderer_runtime.quad_index_buffer);
		bgfx::setUniform(renderer_runtime.quad_transform_uniform, glm::value_ptr(quad_transform));
		bgfx::setUniform(renderer_runtime.quad_color_uniform, &renderer_runtime.quad_color);
		bgfx::setTexture(0, renderer_runtime.quad_texture_uniform, renderer_runtime.quad_texture);
		
		// draw call
		bgfx::submit(0, renderer_runtime.quad_program);
    }
}