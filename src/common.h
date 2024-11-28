#pragma once

#include "base.h"

extern Allocator gpa;

struct BGFX_Vec2 {
	f32 x, y;
};

struct BGFX_Vertex {
	BGFX_Vec2 position, texture_coords;
};

struct BGFX_Color {
	f32 r, g, b, a;
};

u32 bgfx_color(BGFX_Color color);