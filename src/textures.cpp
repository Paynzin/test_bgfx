#include "base.h"
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include <bx/error.h>
#include <bx/allocator.h>

#include "textures.h"

// yes this code is shit, but since it's c++ nothing can be done.
// please stop using c++ stuff
class ConvertAllocator : public bx::AllocatorI
{
	private:
	Allocator m_internal_allocator;

	public:
	ConvertAllocator(Allocator allocator) {
		m_internal_allocator = allocator;
	}
	~ConvertAllocator() {
	}

	void* realloc(void* ptr, usize size, usize align, const c8* file_path, u32 line) {
		void* result = 0;
		
		if (ptr == nullptr || size > 0) {
			result = m_internal_allocator.alloc(size);
		} else if (ptr != nullptr || size > 0) {
			result = m_internal_allocator.realloc(ptr, size);
		} else if (ptr != nullptr || size == 0) {
			m_internal_allocator.free(ptr);
		}
	
		return result;
	}
};

static void bimg_free_image(void* ptr, void* data) {
	BX_UNUSED(ptr);
	bimg::ImageContainer* image_container = (bimg::ImageContainer*) data;
	bimg::imageFree(image_container);
}

BGFX_Texture bgfx_load_texture(String8 path, Allocator allocator) {
	BGFX_Texture texture = {};
	static ConvertAllocator bx_allocator = ConvertAllocator(allocator);
	
	File image = read_entire_file(path, allocator);
	if (image.size == 0) {
		return texture;
	}
	
	bx::Error error;
	bimg::ImageContainer* image_container = bimg::imageParseKtx(&bx_allocator, (void*) image.data, image.size, &error);
	const bgfx::Memory* image_data = bgfx::makeRef(image_container->m_data, image_container->m_size, bimg_free_image, image_container);
	
	allocator.free(image.data);
	
	texture.width = image_container->m_width;
	texture.height = image_container->m_height;
	texture.has_mips = (image_container->m_numMips > 1);
	texture.layers = image_container->m_numLayers;
	texture.format = bgfx::TextureFormat::Enum(image_container->m_format);
	texture.data = image_data;

	return texture;
}