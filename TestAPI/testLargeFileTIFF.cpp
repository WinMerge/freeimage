// FreeImage.h supplies Windows bitmap types and defines _WINDOWS_ when the
// SDK has not been included. Include the SDK first for sparse-file controls.
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#endif

#include "FreeImage.h"
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <vector>

#ifdef _WIN32
#include <io.h>
#define fi_fseeko _fseeki64
#else
#include <sys/types.h>
#define fi_fseeko fseeko
static_assert(sizeof(off_t) >= 8, "The sparse fixture writer needs 64-bit file offsets");
#endif

// FreeImage 3 external plugins exchange exactly 16 callback pointers. Appending
// IO64 callbacks here would let a new plugin overwrite an older host's buffer.
static_assert(sizeof(Plugin) == 16 * sizeof(FI_FormatProc), "Preserve the FreeImage 3 plugin ABI");
static_assert(offsetof(Plugin, supports_no_pixels_proc) == 15 * sizeof(FI_FormatProc),
              "Preserve the last legacy plugin callback offset");

struct Region {
    uint64_t offset;
    std::vector<unsigned char> bytes;
};

// A tiny allocation represents a sparse TIFF whose logical positions exceed
// 32-bit limits. No test image or multi-gigabyte buffer is checked into the repo.
struct SparseInput {
    std::vector<Region> regions;
    uint64_t position = 0;
    uint64_t length = 0;
    uint64_t highestRead = 0;
    bool failRead = false;
    bool failSeek = false;
    bool failTell = false;
    bool overreportRead = false;
};

static void put(std::vector<unsigned char> &bytes, uint64_t value, unsigned width) {
    for (unsigned i = 0; i < width; ++i) bytes.push_back((unsigned char)(value >> (8 * i)));
}

static void entry(std::vector<unsigned char> &bytes, uint16_t tag, uint16_t type, uint64_t count, uint64_t value) {
    put(bytes, tag, 2);
    put(bytes, type, 2);
    put(bytes, count, 8);
    put(bytes, value, 8);
}

static std::vector<unsigned char> directory(uint64_t pixel, uint64_t exif = 0, uint64_t thumbnail = 0) {
    std::vector<unsigned char> bytes;
    put(bytes, 10 + (exif ? 1 : 0) + (thumbnail ? 1 : 0), 8);
    entry(bytes, 256, 4, 1, 1);                 // Width
    entry(bytes, 257, 4, 1, 1);                 // Height
    entry(bytes, 258, 3, 3, 0x000800080008ULL); // BitsPerSample
    entry(bytes, 259, 3, 1, 1);                 // Uncompressed
    entry(bytes, 262, 3, 1, 2);                 // RGB
    entry(bytes, 273, 16, 1, pixel);            // LONG8 StripOffsets
    entry(bytes, 277, 3, 1, 3);                 // SamplesPerPixel
    entry(bytes, 278, 4, 1, 1);                 // RowsPerStrip
    entry(bytes, 279, 16, 1, 3);                // StripByteCounts
    entry(bytes, 284, 3, 1, 1);                 // Contiguous
    if (thumbnail) entry(bytes, 330, 18, 1, thumbnail); // IFD8 SubIFD
    if (exif) entry(bytes, 34665, 18, 1, exif);         // IFD8 ExifIFD
    put(bytes, 0, 8);
    return bytes;
}

static SparseInput makeInput(uint64_t ifdOffset, uint64_t pixelOffset) {
    SparseInput input;
    std::vector<unsigned char> header = {'I', 'I', 43, 0, 8, 0, 0, 0};
    put(header, ifdOffset, 8);
    const uint64_t exifOffset = pixelOffset + 64;
    const uint64_t textOffset = pixelOffset + 128;
    const uint64_t thumbnailOffset = pixelOffset + 256;
    const uint64_t thumbnailPixel = pixelOffset + 512;
    std::vector<unsigned char> exif;
    put(exif, 2, 8);
    entry(exif, 36864, 7, 4, 0x31333230); // ExifVersion "0231"
    entry(exif, 37510, 7, 17, textOffset); // UserComment, stored out of line
    put(exif, 0, 8);
    const unsigned char text[] = "ASCII\0\0\0IO64test";
    input.regions = {
        {0, header},
        {ifdOffset, directory(pixelOffset, exifOffset, thumbnailOffset)},
        {pixelOffset, {0x12, 0x34, 0x56}},
        {exifOffset, exif},
        {textOffset, std::vector<unsigned char>(text, text + sizeof(text))},
        {thumbnailOffset, directory(thumbnailPixel)},
        {thumbnailPixel, {0xab, 0xcd, 0xef}}
    };
    input.length = thumbnailPixel + 3;
    return input;
}

static uint64_t DLL_CALLCONV read64(void *buffer, uint64_t size, uint64_t count, fi_handle handle) {
    SparseInput &input = *static_cast<SparseInput *>(handle);
    if (input.failRead || size == 0 || size > SIZE_MAX || count > SIZE_MAX / size || input.position > input.length) return 0;
    if (input.overreportRead) return count + 1;
    const uint64_t elements = std::min(count, (input.length - input.position) / size);
    const uint64_t bytes = elements * size;
    std::memset(buffer, 0, (size_t)bytes);
    for (const Region &region : input.regions) {
        const uint64_t first = std::max(input.position, region.offset);
        const uint64_t last = std::min(input.position + bytes, region.offset + region.bytes.size());
        if (first < last) std::memcpy(static_cast<unsigned char *>(buffer) + (size_t)(first - input.position),
                                    region.bytes.data() + (size_t)(first - region.offset), (size_t)(last - first));
    }
    input.position += bytes;
    input.highestRead = std::max(input.highestRead, input.position);
    return elements;
}

static int DLL_CALLCONV seek64(fi_handle handle, int64_t offset, int origin) {
    SparseInput &input = *static_cast<SparseInput *>(handle);
    if (input.failSeek) return -1;
    const uint64_t base = origin == SEEK_SET ? 0 : origin == SEEK_CUR ? input.position : input.length;
    if (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END) return -1;
    if (offset < 0) {
        const uint64_t magnitude = (uint64_t)(-(offset + 1)) + 1;
        if (magnitude > base) return -1;
        input.position = base - magnitude;
    } else {
        if ((uint64_t)offset > INT64_MAX - base) return -1;
        input.position = base + (uint64_t)offset;
    }
    return 0;
}

static int64_t DLL_CALLCONV tell64(fi_handle handle) {
    SparseInput &input = *static_cast<SparseInput *>(handle);
    return input.failTell ? -1 : (int64_t)input.position;
}

static bool checkImage(FIBITMAP *dib, bool headerOnly = false) {
    RGBQUAD color = {};
    bool passed = dib && FreeImage_GetWidth(dib) == 1 && FreeImage_GetHeight(dib) == 1;
    if (passed && headerOnly) {
        passed = !FreeImage_HasPixels(dib);
    } else if (passed) {
        passed = FreeImage_GetPixelColor(dib, 0, 0, &color) &&
            color.rgbRed == 0x12 && color.rgbGreen == 0x34 && color.rgbBlue == 0x56;
        FITAG *tag = NULL;
        passed = passed && FreeImage_GetMetadata(FIMD_EXIF_EXIF, dib, "UserComment", &tag) &&
            FreeImage_GetTagLength(tag) == 17 &&
            std::memcmp(FreeImage_GetTagValue(tag), "ASCII\0\0\0IO64test", 17) == 0;
        FIBITMAP *thumbnail = FreeImage_GetThumbnail(dib);
        passed = passed && thumbnail && FreeImage_GetPixelColor(thumbnail, 0, 0, &color) &&
            color.rgbRed == 0xab && color.rgbGreen == 0xcd && color.rgbBlue == 0xef;
    }
    if (!passed) {
        FITAG *tag = NULL;
        FreeImage_GetMetadata(FIMD_EXIF_EXIF, dib, "UserComment", &tag);
        std::fprintf(stderr, "Image check failed: dib=%p, size=%ux%u, Exif=%s, thumbnail=%p\n",
            (void *)dib, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),
            tag ? (const char *)FreeImage_GetTagValue(tag) : "missing", (void *)FreeImage_GetThumbnail(dib));
    }
    if (dib) FreeImage_Unload(dib);
    return passed;
}

static bool expectRejected(FIBITMAP *dib) {
    if (!dib) return true;
    FreeImage_Unload(dib);
    return false;
}

static bool checkCallbacks(uint64_t ifdOffset, uint64_t pixelOffset) {
    SparseInput input = makeInput(ifdOffset, pixelOffset);
    FreeImageIO64 io = {read64, NULL, seek64, tell64}; // Intentionally read-only.
    if (!checkImage(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input)) || input.highestRead <= pixelOffset) return false;
    input.position = 0;
    return checkImage(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input, FIF_LOAD_NOPIXELS), true);
}

static bool checkInvalidCallbacks() {
    SparseInput input = makeInput(16, 1024);
    FreeImageIO64 io = {read64, NULL, seek64, tell64};
    FreeImageIO64 missing = {};
    if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &missing, &input))) return false;
    missing = io; missing.read_proc = NULL;
    if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &missing, &input))) return false;
    missing = io; missing.seek_proc = NULL;
    if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &missing, &input))) return false;
    missing = io; missing.tell_proc = NULL;
    if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &missing, &input))) return false;
    if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, NULL, &input)) ||
        !expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &io, NULL)) ||
        !expectRejected(FreeImage_LoadFromHandle64(FIF_UNKNOWN, &io, &input)) ||
        !expectRejected(FreeImage_LoadFromHandle64(FIF_PNG, &io, &input)) ||
        !expectRejected(FreeImage_Load64(FIF_TIFF, NULL))) return false;
    const BOOL enabled = FreeImage_IsPluginEnabled(FIF_TIFF);
    FreeImage_SetPluginEnabled(FIF_TIFF, FALSE);
    const bool disabledRejected = expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input));
    FreeImage_SetPluginEnabled(FIF_TIFF, enabled);
    if (!disabledRejected) return false;
    for (unsigned failure = 0; failure < 4; ++failure) {
        input.position = 0;
        input.failRead = failure == 0;
        input.failSeek = failure == 1;
        input.failTell = failure == 2;
        input.overreportRead = failure == 3;
        if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input))) return false;
    }
    input = makeInput(16, 1024);
    // An unsigned TIFF absolute offset above INT64_MAX is not a negative seek.
    std::fill(input.regions[0].bytes.begin() + 8, input.regions[0].bytes.end(), 0xff);
    return expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input));
}

static unsigned DLL_CALLCONV readLegacy(void *buffer, unsigned size, unsigned count, fi_handle handle) {
    return (unsigned)read64(buffer, size, count, handle);
}

static int DLL_CALLCONV seekLegacy(fi_handle handle, long offset, int origin) {
    return seek64(handle, offset, origin);
}

static long DLL_CALLCONV tellLegacy(fi_handle handle) {
    const int64_t position = tell64(handle);
    return position <= std::numeric_limits<long>::max() ? (long)position : -1;
}

static bool checkLegacy() {
    SparseInput input = makeInput(16, 1024);
    FreeImageIO io = {readLegacy, NULL, seekLegacy, tellLegacy};
    return checkImage(FreeImage_LoadFromHandle(FIF_TIFF, &io, &input));
}

static bool checkSparseFile() {
    const char *path = "large-file-tiff-test.tif";
    FILE *file = std::fopen(path, "wb");
    if (!file) return false;
#ifdef _WIN32
    // Seeking past EOF otherwise materializes gigabytes of zeroes on Windows.
    DWORD returned = 0;
    const HANDLE native = (HANDLE)_get_osfhandle(_fileno(file));
    if (!DeviceIoControl(native, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &returned, NULL)) {
        std::fprintf(stderr, "Cannot mark TIFF fixture sparse: %lu\n", GetLastError());
        std::fclose(file);
        std::remove(path);
        return false;
    }
#endif
    const SparseInput input = makeInput(16, 0x80000010ULL);
    bool written = true;
    for (const Region &region : input.regions) {
        if (fi_fseeko(file, (int64_t)region.offset, SEEK_SET) != 0 ||
            std::fwrite(region.bytes.data(), 1, region.bytes.size(), file) != region.bytes.size()) {
            written = false;
            break;
        }
    }
    written = std::fclose(file) == 0 && written;
    const bool passed = written && checkImage(FreeImage_Load64(FIF_TIFF, path));
    std::remove(path);
    return passed;
}

static void message(FREE_IMAGE_FORMAT, const char *text) {
    std::fprintf(stderr, "TIFF: %s\n", text);
}

int main() {
    SparseInput input;
    FreeImageIO64 io = {read64, NULL, seek64, tell64};
    if (!expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input))) return 1;
    FreeImage_Initialise(FALSE);
    FreeImage_SetOutputMessage(message);
    const struct { const char *name; bool passed; } results[] = {
        {"callbacks above 2 GiB, Exif and thumbnail", checkCallbacks(16, 0x80000010ULL)},
        {"IFD/pixels above 4 GiB, Exif and thumbnail", checkCallbacks(0x100000010ULL, 0x100001000ULL)},
        {"invalid callbacks, failed I/O and disabled plugin", checkInvalidCallbacks()},
        {"unchanged legacy TIFF callback loading", checkLegacy()},
        {"default file I/O beyond 2 GiB", checkSparseFile()}
    };
    bool passed = true;
    for (const auto &result : results) {
        std::printf("%s: %s\n", result.name, result.passed ? "PASS" : "FAIL");
        passed = passed && result.passed;
    }
    FreeImage_DeInitialise();
    passed = expectRejected(FreeImage_LoadFromHandle64(FIF_TIFF, &io, &input)) && passed;
    return passed ? 0 : 1;
}
