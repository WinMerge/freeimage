#include "FreeImage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

static void Require(bool ok, const char *what) {
    if(!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
static void Test(const char *filename, FREE_IMAGE_FORMAT format, bool alpha) {
    std::ifstream input(filename, std::ios::binary);
    Require(bool(input), "open fixture");
    std::vector<BYTE> bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    Require(!bytes.empty(), "read fixture");
    BYTE profile[128] = {};
    profile[3] = 128;
    std::memcpy(profile + 36, "acsp", 4);
    FIMEMORY *memory = FreeImage_OpenMemory(bytes.data(), static_cast<DWORD>(bytes.size()));
    Require(FreeImage_GetFileTypeFromMemory(memory) == format, "format detection");
    Require(FreeImage_TellMemory(memory) == 0, "validation preserves position");
    FIBITMAP *dib = FreeImage_LoadFromMemory(format, memory);
    Require(dib && FreeImage_GetWidth(dib) == 32 && FreeImage_GetHeight(dib) == 24, "decode dimensions");
    Require(FreeImage_GetBPP(dib) == (alpha ? 32U : 24U), "pixel depth");
    for(int y = 3; y < 24; y += 17) {
        const BYTE *p = FreeImage_GetScanLine(dib, 23 - y) + 4 * (alpha ? 4 : 3);
        Require(std::abs(int(p[FI_RGBA_RED]) - (y < 12 ? 220 : 30)) < 35, "red and orientation");
        Require(std::abs(int(p[FI_RGBA_BLUE]) - (y < 12 ? 30 : 220)) < 35, "blue and orientation");
        if(alpha) Require(std::abs(int(p[FI_RGBA_ALPHA]) - (y < 12 ? 255 : 100)) < 4, "alpha");
    }
    FIICCPROFILE *icc = FreeImage_GetICCProfile(dib);
    Require(icc && icc->size == sizeof(profile) && !std::memcmp(icc->data, profile, sizeof(profile)), "ICC profile");
    FreeImage_Unload(dib);
    FreeImage_SeekMemory(memory, 0, SEEK_SET);
    dib = FreeImage_LoadFromMemory(format, memory, FIF_LOAD_NOPIXELS);
    Require(dib && !FreeImage_HasPixels(dib) && FreeImage_GetWidth(dib) == 32, "header-only");
    FreeImage_Unload(dib); FreeImage_CloseMemory(memory);
    std::vector<BYTE> prefixed(17, 0);
    prefixed.insert(prefixed.end(), bytes.begin(), bytes.end());
    memory = FreeImage_OpenMemory(prefixed.data(), static_cast<DWORD>(prefixed.size()));
    FreeImage_SeekMemory(memory, 17, SEEK_SET);
    Require(FreeImage_GetFileTypeFromMemory(memory) == format && FreeImage_TellMemory(memory) == 17, "offset detection");
    dib = FreeImage_LoadFromMemory(format, memory);
    Require(dib && FreeImage_GetHeight(dib) == 24, "nonzero stream origin");
    FreeImage_Unload(dib); FreeImage_CloseMemory(memory);
    memory = FreeImage_OpenMemory(bytes.data(), 12);
    Require(FreeImage_LoadFromMemory(format, memory) == NULL, "truncated input rejected");
    FreeImage_CloseMemory(memory);
}
int main() {
    FreeImage_Initialise();
    Require(FreeImage_GetFIFFromFormat("HEIF") == FIF_HEIF, "HEIF index");
    Require(FreeImage_GetFIFFromFormat("AVIF") == FIF_AVIF, "AVIF index");
    Require(!FreeImage_FIFSupportsWriting(FIF_HEIF) && !FreeImage_FIFSupportsWriting(FIF_AVIF), "read-only");
    Test("rgb.heic", FIF_HEIF, false);
    Test("rgba.heic", FIF_HEIF, true);
    Test("rgb.avif", FIF_AVIF, false);
    Test("rgba.avif", FIF_AVIF, true);
    FreeImage_DeInitialise();
    std::puts("HEIF/AVIF RGB, alpha, orientation, header-only and truncated-input tests passed");
}
