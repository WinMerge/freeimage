#include "FreeImage.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// 8421504 * 255 == 2147483520, which still fits in a signed 32-bit cursor.
// One more delta of 255 wraps that cursor negative (GHSA-7pp5-4w3w-437c).
static const unsigned kWrapDeltas = 8421505u;

static void require(bool condition, const char *message) {
	if(!condition) {
		std::fprintf(stderr, "BMP RLE cursor test failed: %s\n", message);
		std::exit(1);
	}
}

static void append_u16(std::vector<BYTE> &out, unsigned value) {
	out.push_back(static_cast<BYTE>(value));
	out.push_back(static_cast<BYTE>(value >> 8));
}

static void append_u32(std::vector<BYTE> &out, unsigned value) {
	out.push_back(static_cast<BYTE>(value));
	out.push_back(static_cast<BYTE>(value >> 8));
	out.push_back(static_cast<BYTE>(value >> 16));
	out.push_back(static_cast<BYTE>(value >> 24));
}

static void append_deltas(std::vector<BYTE> &rle, unsigned count, BYTE dx, BYTE dy) {
	rle.reserve(rle.size() + static_cast<size_t>(count) * 4u + 8u);
	for(unsigned i = 0; i < count; ++i) {
		rle.push_back(0);
		rle.push_back(2);
		rle.push_back(dx);
		rle.push_back(dy);
	}
}

static std::vector<BYTE> make_rle_bmp(unsigned width, unsigned height, unsigned bit_count,
		unsigned compression, const std::vector<BYTE> &rle) {
	require(bit_count == 4 || bit_count == 8, "unsupported RLE bit count");
	const unsigned palette_entries = bit_count == 4 ? 16u : 256u;
	const unsigned off_bits = 14u + 40u + palette_entries * 4u;
	const unsigned file_size = off_bits + static_cast<unsigned>(rle.size());

	std::vector<BYTE> file;
	file.reserve(file_size);
	file.push_back('B');
	file.push_back('M');
	append_u32(file, file_size);
	append_u16(file, 0);
	append_u16(file, 0);
	append_u32(file, off_bits);
	append_u32(file, 40);
	append_u32(file, width);
	append_u32(file, height);
	append_u16(file, 1);
	append_u16(file, bit_count);
	append_u32(file, compression);
	append_u32(file, static_cast<unsigned>(rle.size()));
	append_u32(file, 0);
	append_u32(file, 0);
	append_u32(file, 0);
	append_u32(file, 0);
	file.resize(off_bits, 0);
	file.insert(file.end(), rle.begin(), rle.end());
	return file;
}

static FIBITMAP *load_bmp(const std::vector<BYTE> &file) {
	FIMEMORY *memory = FreeImage_OpenMemory(const_cast<BYTE *>(file.data()),
		static_cast<DWORD>(file.size()));
	require(memory != NULL, "could not open in-memory BMP");
	FIBITMAP *image = FreeImage_LoadFromMemory(FIF_BMP, memory, 0);
	FreeImage_CloseMemory(memory);
	return image;
}

static void expect_indices(FIBITMAP *image, const BYTE *pixels, unsigned width, unsigned height,
		const char *name) {
	require(image != NULL, name);
	require(FreeImage_GetWidth(image) == width && FreeImage_GetHeight(image) == height, name);
	for(unsigned y = 0; y < height; ++y) {
		for(unsigned x = 0; x < width; ++x) {
			BYTE index = 0xFF;
			require(FreeImage_GetPixelIndex(image, x, y, &index) != FALSE, name);
			if(index != pixels[y * width + x]) {
				std::fprintf(stderr, "BMP RLE cursor test failed: %s pixel (%u,%u) is %u, expected %u\n",
					name, x, y, index, pixels[y * width + x]);
				std::exit(1);
			}
		}
	}
}

static void expect_rejected(const std::vector<BYTE> &file, const char *name) {
	FIBITMAP *image = load_bmp(file);
	if(image != NULL) {
		FreeImage_Unload(image);
		std::fprintf(stderr, "BMP RLE cursor test failed: %s was accepted\n", name);
		std::exit(1);
	}
}

static void test_rle8_pixels() {
	const BYTE rle[] = {
		2, 0x11,
		0, 2, 1, 0,
		1, 0x22,
		0, 0,
		0, 3, 3, 4, 5, 0,
		0, 1
	};
	const BYTE pixels[] = {
		0x11, 0x11, 0x00, 0x22,
		3, 4, 5, 0
	};
	std::vector<BYTE> file = make_rle_bmp(4, 2, 8, 1, std::vector<BYTE>(rle, rle + sizeof(rle)));
	FIBITMAP *image = load_bmp(file);
	expect_indices(image, pixels, 4, 2, "rle8-pixels");
	FreeImage_Unload(image);
}

static void test_rle8_eob_after_eol() {
	const BYTE rle[] = { 4, 7, 0, 0, 0, 1 };
	const BYTE pixels[] = { 7, 7, 7, 7 };
	std::vector<BYTE> file = make_rle_bmp(4, 1, 8, 1, std::vector<BYTE>(rle, rle + sizeof(rle)));
	FIBITMAP *image = load_bmp(file);
	expect_indices(image, pixels, 4, 1, "rle8-eob-after-eol");
	FreeImage_Unload(image);
}

static void test_rle8_delta_past_then_eob() {
	const BYTE rle[] = { 0, 2, 5, 0, 0, 1 };
	const BYTE pixels[] = { 0, 0, 0, 0 };
	std::vector<BYTE> file = make_rle_bmp(4, 1, 8, 1, std::vector<BYTE>(rle, rle + sizeof(rle)));
	FIBITMAP *image = load_bmp(file);
	expect_indices(image, pixels, 4, 1, "rle8-delta-past-then-eob");
	FreeImage_Unload(image);
}

static void test_rle8_past_width() {
	const BYTE rle[] = { 0, 2, 255, 0, 1, 0xAA, 0, 1 };
	expect_rejected(make_rle_bmp(4, 1, 8, 1, std::vector<BYTE>(rle, rle + sizeof(rle))),
		"rle8-past-width");
}

static void test_rle8_overflow_x() {
	std::vector<BYTE> rle;
	append_deltas(rle, kWrapDeltas, 255, 0);
	rle.push_back(0xFF);
	rle.push_back(0xAA);
	rle.push_back(0);
	rle.push_back(1);
	expect_rejected(make_rle_bmp(4, 1, 8, 1, rle), "rle8-overflow-x");
}

static void test_rle8_overflow_y() {
	std::vector<BYTE> rle;
	append_deltas(rle, kWrapDeltas, 0, 255);
	rle.push_back(1);
	rle.push_back(0xAA);
	rle.push_back(0);
	rle.push_back(1);
	expect_rejected(make_rle_bmp(4, 1, 8, 1, rle), "rle8-overflow-y");
}

static void test_rle4_pixels() {
	const BYTE rle[] = { 2, 0x10, 0, 2, 1, 0, 1, 0x20, 0, 1 };
	const BYTE pixels[] = { 1, 0, 0, 2 };
	std::vector<BYTE> file = make_rle_bmp(4, 1, 4, 2, std::vector<BYTE>(rle, rle + sizeof(rle)));
	FIBITMAP *image = load_bmp(file);
	expect_indices(image, pixels, 4, 1, "rle4-pixels");
	FreeImage_Unload(image);
}

static void test_rle4_delta_past() {
	// A delta that leaves the temporary buffer stops the decode and keeps the
	// pixels already written. It must not wrap the cursor or fault.
	const BYTE rle[] = { 0, 2, 255, 0, 1, 0x10, 0, 1 };
	const BYTE pixels[] = { 0, 0, 0, 0 };
	std::vector<BYTE> file = make_rle_bmp(4, 1, 4, 2, std::vector<BYTE>(rle, rle + sizeof(rle)));
	FIBITMAP *image = load_bmp(file);
	expect_indices(image, pixels, 4, 1, "rle4-delta-past");
	FreeImage_Unload(image);
}

int main(int argc, char **argv) {
	const char *only = argc > 1 ? argv[1] : NULL;
	struct Case {
		const char *name;
		void (*run)();
	};
	const Case cases[] = {
		{ "rle8-pixels", test_rle8_pixels },
		{ "rle8-eob-after-eol", test_rle8_eob_after_eol },
		{ "rle8-delta-past-then-eob", test_rle8_delta_past_then_eob },
		{ "rle8-past-width", test_rle8_past_width },
		{ "rle8-overflow-x", test_rle8_overflow_x },
		{ "rle8-overflow-y", test_rle8_overflow_y },
		{ "rle4-pixels", test_rle4_pixels },
		{ "rle4-delta-past", test_rle4_delta_past }
	};

	FreeImage_Initialise();
	bool matched = only == NULL;
	for(const Case &entry : cases) {
		if(only != NULL && std::strcmp(only, entry.name) != 0) {
			continue;
		}
		matched = true;
		entry.run();
		std::printf("BMP RLE %s passed\n", entry.name);
	}
	FreeImage_DeInitialise();
	require(matched, "unknown test case");
	return 0;
}
