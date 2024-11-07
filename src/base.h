#pragma once

#include <stdint.h>
#include <stddef.h>

// one byte types
typedef unsigned char byte;
typedef char c8;
typedef bool b8;
// unsinged ints
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
// signed ints 
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
// size ptr
typedef size_t usize;
// floats
typedef float f32;
typedef double f64;
typedef long double f80;

// strings
#define DEFAULT_STRING_SIZE 15
struct String8 {
	c8* data;
	u32 len;
	usize reserved;

	String8 operator=(const c8* in_string);
};

struct Allocator; // required by 'set_string_allocator()'
void set_string_allocator(Allocator allocator);
String8 create_string(u32 reserve = DEFAULT_STRING_SIZE);
String8 create_string_from(const c8* in_string); // NOTE: the input string has to be null terminated
void assign_string(String8* string, const c8* in_string); // NOTE: the input string has to be null terminated
void destroy_string(String8* string);

// custom allocators
struct Allocator {
	void* (*alloc)(usize size);
	void* (*realloc)(void* ptr, usize size);
	void (*free)(void* ptr);
};

// utils
struct File {
	byte* data;
	usize size;
};

File read_entire_file(String8 path, Allocator allocator);
String8 read_entire_file_as_string(String8 path);