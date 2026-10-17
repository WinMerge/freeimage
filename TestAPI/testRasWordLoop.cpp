#include "FreeImage.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

static void append_be32(std::vector<BYTE> &file, unsigned value) {
	file.push_back(static_cast<BYTE>(value >> 24));
	file.push_back(static_cast<BYTE>(value >> 16));
	file.push_back(static_cast<BYTE>(value >> 8));
	file.push_back(static_cast<BYTE>(value));
}

int main() {
	const unsigned width = 70000;
	const unsigned height = 1;
	const unsigned data_size = width * 3;
	std::vector<BYTE> file;
	file.reserve(32 + data_size);
	append_be32(file, 0x59A66A95); // Sun raster magic
	append_be32(file, width);
	append_be32(file, height);
	append_be32(file, 24); // depth
	append_be32(file, data_size);
	append_be32(file, 1); // RT_STANDARD
	append_be32(file, 0); // RMT_NONE
	append_be32(file, 0); // no color map
	file.insert(file.end(), data_size, 0);

	FreeImage_Initialise();
	FIMEMORY *memory = FreeImage_OpenMemory(file.data(), static_cast<DWORD>(file.size()));
	if(memory == NULL) {
		std::fprintf(stderr, "RAS word-loop test failed: could not open memory stream\n");
		FreeImage_DeInitialise();
		return 1;
	}

	FIBITMAP *image = FreeImage_LoadFromMemory(FIF_RAS, memory, 0);
	const bool valid = image != NULL && FreeImage_GetWidth(image) == width &&
		FreeImage_GetHeight(image) == height && FreeImage_GetBPP(image) == 24;
	if(image != NULL) {
		FreeImage_Unload(image);
	}
	FreeImage_CloseMemory(memory);
	FreeImage_DeInitialise();
	if(!valid) {
		std::fprintf(stderr, "RAS word-loop test failed: large valid raster did not decode\n");
		return 1;
	}
	return 0;
}
