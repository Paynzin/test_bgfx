#pragma once

#include "base.h"
#include <bgfx/bgfx.h>

#include "common.h"

struct Renderer_Runtime {
	u64 performance_frequency;
	f32 delta;
	f32 fps;
	
	BGFX_Color clear_color;
	BGFX_Color quad_color;
	f32 quad_scale;
	
	bgfx::VertexBufferHandle quad_vertex_buffer;
	bgfx::IndexBufferHandle quad_index_buffer;
	bgfx::ProgramHandle quad_program;
	bgfx::TextureHandle quad_texture;
	bgfx::UniformHandle quad_texture_uniform;
	bgfx::UniformHandle quad_transform_uniform;
	bgfx::UniformHandle quad_color_uniform;
};

void renderer_initialize();
void renderer_render(s32 win_width, s32 win_height);