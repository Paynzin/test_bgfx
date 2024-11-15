#pragma once

#include "base.h"
#include <bgfx/bgfx.h>

struct BGFX_Texture {
	u16 width, height;
	b8 has_mips;
	u16 layers;
	bgfx::TextureFormat::Enum format;
	const bgfx::Memory* data;
};

// NOTE: all textures must be in ktx format
BGFX_Texture bgfx_load_texture(String8 path, Allocator allocator);