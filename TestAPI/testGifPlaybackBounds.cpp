#include "FreeImage.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

static void append_u16(std::vector<BYTE> &gif, unsigned value) {
	gif.push_back(static_cast<BYTE>(value));
	gif.push_back(static_cast<BYTE>(value >> 8));
}

static void append_graphic_control(std::vector<BYTE> &gif, BYTE disposal) {
	const BYTE block[] = {0x21, 0xF9, 0x04, disposal, 0, 0, 0, 0};
	gif.insert(gif.end(), block, block + sizeof(block));
}

static void append_frame(std::vector<BYTE> &gif, unsigned left, unsigned top,
		unsigned width, unsigned height, const std::vector<BYTE> &compressed) {
	gif.push_back(0x2C);
	append_u16(gif, left);
	append_u16(gif, top);
	append_u16(gif, width);
	append_u16(gif, height);
	gif.push_back(0); // no local color table
	gif.push_back(2); // LZW minimum code size
	gif.push_back(static_cast<BYTE>(compressed.size()));
	gif.insert(gif.end(), compressed.begin(), compressed.end());
	gif.push_back(0); // end of image data
}

static std::vector<BYTE> make_gif() {
	std::vector<BYTE> gif = {
		'G', 'I', 'F', '8', '9', 'a',
		10, 0, 10, 0, 0x80, 0, 0, // 10x10 logical screen, two-entry global palette
		0, 0, 0, 255, 255, 255
	};

	// A 5x1 frame begins at x=8, beyond the 10-pixel logical screen.
	append_graphic_control(gif, 0x08); // disposal to background
	append_frame(gif, 8, 0, 5, 1,
		std::vector<BYTE>{0x0C, 0xC3, 0x30, 0x4C, 0x01});

	// Page 1 clears frame 0's partially visible rectangle and then places a
	// one-pixel frame entirely outside the logical screen.
	append_graphic_control(gif, 0x08);
	append_frame(gif, 60000, 1, 1, 1, std::vector<BYTE>{0x4C, 0x01});

	// Page 2 clears a prior frame whose left edge is far beyond the canvas.
	append_graphic_control(gif, 0);
	append_frame(gif, 0, 2, 1, 1, std::vector<BYTE>{0x4C, 0x01});
	gif.push_back(0x3B);
	return gif;
}

static void require(bool condition, const char *message) {
	if(!condition) {
		std::fprintf(stderr, "GIF playback bounds test failed: %s\n", message);
		std::exit(1);
	}
}

int main() {
	FreeImage_Initialise();
	std::vector<BYTE> bytes = make_gif();
	FIMEMORY *memory = FreeImage_OpenMemory(bytes.data(), static_cast<DWORD>(bytes.size()));
	require(memory != NULL, "could not open in-memory GIF");

	FIMULTIBITMAP *animation = FreeImage_LoadMultiBitmapFromMemory(FIF_GIF, memory, GIF_PLAYBACK);
	require(animation != NULL, "could not load animated GIF");
	require(FreeImage_GetPageCount(animation) == 3, "unexpected frame count");

	for(int page = 0; page < 3; ++page) {
		FIBITMAP *frame = FreeImage_LockPage(animation, page);
		require(frame != NULL, "playback frame failed to load");
		require(FreeImage_GetWidth(frame) == 10 && FreeImage_GetHeight(frame) == 10,
			"playback canvas has the wrong size");
		require(FreeImage_GetBPP(frame) == 32, "playback canvas is not 32-bit");
		FreeImage_UnlockPage(animation, frame, FALSE);
	}

	FreeImage_CloseMultiBitmap(animation, 0);
	FreeImage_CloseMemory(memory);
	FreeImage_DeInitialise();
	return 0;
}
