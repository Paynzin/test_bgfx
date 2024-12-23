#include "base.h"
#include <SDL.h>

#include "common.h"

Allocator gpa = {
	.alloc = SDL_malloc,
	.realloc = SDL_realloc,
	.free = SDL_free
};

static inline u8 clamp_and_scale(f32 value) {
	return (u8) (value < 0.0f ? 0 : (value > 1.0f ? 255 : value * 255.0f));
}

u32 bgfx_color(BGFX_Color color) {
	u8 r = clamp_and_scale(color.r);
	u8 g = clamp_and_scale(color.g);
	u8 b = clamp_and_scale(color.b);
	u8 a = clamp_and_scale(color.a);
	
	return ((u32) r << 24) | ((u32) g << 16) | ((u32) b << 8) | (u32) a;
}