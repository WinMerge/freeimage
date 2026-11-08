// Decoder for the individually downloaded test-fixtures-io64-v1 assets.
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include "FreeImage.h"
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <limits>
#ifndef _WIN32
#include <sys/types.h>
static_assert(sizeof(off_t) >= 8, "Release fixtures require wide file offsets");
#endif

static int seekFile(FILE *file, int64_t offset, int origin) {
#ifdef _WIN32
    return _fseeki64(file, offset, origin);
#else
    return fseeko(file, (off_t)offset, origin);
#endif
}

static int64_t DLL_CALLCONV tell64(fi_handle handle) {
#ifdef _WIN32
    return _ftelli64(static_cast<FILE *>(handle));
#else
    return ftello(static_cast<FILE *>(handle));
#endif
}

static int DLL_CALLCONV seek64(fi_handle handle, int64_t offset, int origin) {
    return seekFile(static_cast<FILE *>(handle), offset, origin);
}

static uint64_t DLL_CALLCONV read64(void *buffer, uint64_t size, uint64_t count, fi_handle handle) {
    if (!size || size > SIZE_MAX || count > SIZE_MAX / size) return 0;
    return std::fread(buffer, (size_t)size, (size_t)count, static_cast<FILE *>(handle));
}

static unsigned DLL_CALLCONV readLegacy(void *buffer, unsigned size, unsigned count, fi_handle handle) {
    return (unsigned)read64(buffer, size, count, handle);
}

static int DLL_CALLCONV seekLegacy(fi_handle handle, long offset, int origin) {
    return seek64(handle, offset, origin);
}

static long DLL_CALLCONV tellLegacy(fi_handle handle) {
    const int64_t position = tell64(handle);
    return position < 0 || position > (std::numeric_limits<long>::max)() ? -1 : (long)position;
}

static int check(FIBITMAP *dib, bool reject, bool known, bool metadata, bool headerOnly = false) {
    int result = 0;
    if (reject) {
        if (dib) {
            std::fprintf(stderr, "Malformed input returned a bitmap instead of being rejected\n");
            result = known ? 3 : 1;
        }
    } else {
        RGBQUAD color = {};
        bool passed = dib && FreeImage_GetWidth(dib) == 1 && FreeImage_GetHeight(dib) == 1;
        if (passed && headerOnly) {
            passed = !FreeImage_HasPixels(dib);
        } else if (passed) {
            passed = FreeImage_GetPixelColor(dib, 0, 0, &color) &&
                color.rgbRed == 0x12 && color.rgbGreen == 0x34 && color.rgbBlue == 0x56;
            if (passed && metadata) {
                FITAG *tag = NULL;
                FIBITMAP *thumbnail = FreeImage_GetThumbnail(dib);
                passed = FreeImage_GetMetadata(FIMD_EXIF_EXIF, dib, "UserComment", &tag) && tag &&
                    FreeImage_GetTagLength(tag) == 17 &&
                    std::memcmp(FreeImage_GetTagValue(tag), "ASCII\0\0\0IO64test", 17) == 0 &&
                    thumbnail && FreeImage_GetWidth(thumbnail) == 1 && FreeImage_GetHeight(thumbnail) == 1 &&
                    FreeImage_GetPixelColor(thumbnail, 0, 0, &color) &&
                    color.rgbRed == 0x12 && color.rgbGreen == 0x34 && color.rgbBlue == 0x56;
            }
        }
        if (!passed) {
            std::fprintf(stderr, "Decoded pixels, dimensions, header or metadata did not match the fixture\n");
            result = 1;
        }
    }
    FreeImage_Unload(dib);
    return result;
}

static int decode(FILE *file, const char *filename, FREE_IMAGE_FORMAT format,
                  bool reject, bool known, bool metadata) {
    FreeImageIO64 io64 = {read64, NULL, seek64, tell64};
    FreeImageIO legacy = {readLegacy, NULL, seekLegacy, tellLegacy};
    int result = 0;
    if (filename) {
        result = check(format == FIF_TIFF ? FreeImage_Load64(format, filename) : FreeImage_Load(format, filename),
                       reject, known, metadata);
        if (result == 1) return result;
    }
    if (seekFile(file, 0, SEEK_SET) != 0) return 1;
    const int streamed = check(format == FIF_TIFF ? FreeImage_LoadFromHandle64(format, &io64, file) :
                              FreeImage_LoadFromHandle(format, &legacy, file), reject, known, metadata);
    if (streamed == 1) return streamed;
    if (!reject && format == FIF_TIFF) {
        if (seekFile(file, 0, SEEK_SET) != 0) return 1;
        if (check(FreeImage_LoadFromHandle64(format, &io64, file, FIF_LOAD_NOPIXELS),
                  false, false, false, true) != 0) return 1;
    }
    return result == 3 || streamed == 3 ? 3 : 0;
}

#ifdef _WIN32
int wmain(int argc, wchar_t **argv) {
    if (argc != 5) return 2;
    const FREE_IMAGE_FORMAT format = !std::wcscmp(argv[1], L"TIFF") ? FIF_TIFF :
        !std::wcscmp(argv[1], L"PNG") ? FIF_PNG : !std::wcscmp(argv[1], L"BMP") ? FIF_BMP : FIF_UNKNOWN;
    const bool reject = !std::wcscmp(argv[2], L"reject");
    const bool known = !std::wcscmp(argv[4], L"returns_bitmap");
    const bool metadata = std::wcsstr(argv[3], L"exif-thumbnail") != NULL;
    FILE *file = _wfopen(argv[3], L"rb");
    // UTF-16 paths use the custom stream API; ASCII paths also test Load64.
    char narrow[MAX_PATH];
    bool ascii = std::wcslen(argv[3]) < MAX_PATH;
    for (size_t i = 0; ascii && i <= std::wcslen(argv[3]); ++i) {
        if (argv[3][i] > 127) ascii = false;
        else narrow[i] = (char)argv[3][i];
    }
    const char *filename = ascii ? narrow : NULL;
#else
int main(int argc, char **argv) {
    if (argc != 5) return 2;
    const FREE_IMAGE_FORMAT format = !std::strcmp(argv[1], "TIFF") ? FIF_TIFF :
        !std::strcmp(argv[1], "PNG") ? FIF_PNG : !std::strcmp(argv[1], "BMP") ? FIF_BMP : FIF_UNKNOWN;
    const bool reject = !std::strcmp(argv[2], "reject");
    const bool known = !std::strcmp(argv[4], "returns_bitmap");
    const bool metadata = std::strstr(argv[3], "exif-thumbnail") != NULL;
    FILE *file = std::fopen(argv[3], "rb");
    const char *filename = argv[3];
#endif
    if (!file || format == FIF_UNKNOWN) {
        if (file) std::fclose(file);
        return 1;
    }
    FreeImage_Initialise(FALSE);
    const int result = decode(file, filename, format, reject, known, metadata);
    FreeImage_DeInitialise();
    std::fclose(file);
    return result;
}
