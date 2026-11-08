#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "FreeImage.h"
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#define FUNCTIONS(X) \
    X(Initialise) X(DeInitialise) X(GetVersion) X(SetOutputMessage) \
    X(Allocate) X(Unload) X(GetWidth) X(GetHeight) X(SetPixelColor) X(GetPixelColor) \
    X(OpenMemory) X(CloseMemory) X(AcquireMemory) X(SeekMemory) \
    X(SaveToMemory) X(LoadFromMemory) X(LoadFromHandle) X(GetFileTypeFromMemory)

class API {
public:
#define DECLARE(name) decltype(&FreeImage_##name) name;
    FUNCTIONS(DECLARE)
#undef DECLARE
    explicit API(HMODULE module) {
#define BIND(name) name = reinterpret_cast<decltype(name)>(GetProcAddress(module, "FreeImage_" #name)); \
        if (!name) throw std::runtime_error("Missing export: FreeImage_" #name);
        FUNCTIONS(BIND)
#undef BIND
    }
};

static void require(bool value, const char *message) {
    if (!value) throw std::runtime_error(message);
}

struct Stream {
    const BYTE *data;
    size_t length;
    size_t position = 0;
    unsigned reads = 0;
    unsigned seeks = 0;
};

static unsigned DLL_CALLCONV read(void *buffer, unsigned size, unsigned count, fi_handle handle) {
    Stream &stream = *static_cast<Stream *>(handle);
    ++stream.reads;
    if (size == 0 || count == 0 || stream.position >= stream.length) return 0;
    // Match fread: copy a final partial item, but return only complete items.
    // JXR reads large packets and still consumes the bytes from a short read.
    const size_t bytes = std::min<size_t>(static_cast<size_t>(size) * count, stream.length - stream.position);
    std::memcpy(buffer, stream.data + stream.position, bytes);
    stream.position += bytes;
    return static_cast<unsigned>(bytes / size);
}

static int DLL_CALLCONV seek(fi_handle handle, long offset, int origin) {
    Stream &stream = *static_cast<Stream *>(handle);
    ++stream.seeks;
    if (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END) return -1;
    const int64_t base = origin == SEEK_SET ? 0 : static_cast<int64_t>(origin == SEEK_CUR ? stream.position : stream.length);
    const int64_t next = base + offset;
    if (next < 0 || static_cast<uint64_t>(next) > stream.length) return -1;
    stream.position = static_cast<size_t>(next);
    return 0;
}

static long DLL_CALLCONV tell(fi_handle handle) {
    return static_cast<long>(static_cast<Stream *>(handle)->position);
}

static void message(FREE_IMAGE_FORMAT fif, const char *text) {
    std::fprintf(stderr, "FreeImage[%d]: %s\n", static_cast<int>(fif), text);
}

static bool equal(const RGBQUAD &a, const RGBQUAD &b) {
    return a.rgbRed == b.rgbRed && a.rgbGreen == b.rgbGreen && a.rgbBlue == b.rgbBlue;
}

static void roundtrip(API &api, FIBITMAP *source, FREE_IMAGE_FORMAT fif, const char *name, int flags, bool lossless) {
    std::printf("%s: testing memory/callback round trip\n", name);
    std::unique_ptr<FIMEMORY, decltype(api.CloseMemory)> memory(api.OpenMemory(NULL, 0), api.CloseMemory);
    require(memory != nullptr, "OpenMemory failed");
    require(api.SaveToMemory(fif, source, memory.get(), flags) != FALSE, "Memory encoding failed");
    BYTE *bytes = NULL;
    DWORD length = 0;
    require(api.AcquireMemory(memory.get(), &bytes, &length) != FALSE && length != 0, "AcquireMemory failed");
    require(api.SeekMemory(memory.get(), 0, SEEK_SET) != FALSE, "Memory rewind failed");
    require(api.GetFileTypeFromMemory(memory.get(), 0) == fif, "Encoded signature has wrong format");
    require(api.SeekMemory(memory.get(), 0, SEEK_SET) != FALSE, "Memory rewind failed");
    std::unique_ptr<FIBITMAP, decltype(api.Unload)> decoded(api.LoadFromMemory(fif, memory.get(), 0), api.Unload);
    require(decoded != nullptr, "Memory decoding failed");
    Stream stream{bytes, length};
    FreeImageIO io = {read, NULL, seek, tell};
    std::unique_ptr<FIBITMAP, decltype(api.Unload)> callbacks(api.LoadFromHandle(fif, &io, &stream, 0), api.Unload);
    require(callbacks != nullptr && stream.reads > 0, "Callback decoding failed");
    const unsigned width = api.GetWidth(source), height = api.GetHeight(source);
    require(api.GetWidth(decoded.get()) == width && api.GetHeight(decoded.get()) == height &&
            api.GetWidth(callbacks.get()) == width && api.GetHeight(callbacks.get()) == height, "Decoded dimensions differ");
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            RGBQUAD original{}, a{}, b{};
            require(api.GetPixelColor(source, x, y, &original) && api.GetPixelColor(decoded.get(), x, y, &a) &&
                    api.GetPixelColor(callbacks.get(), x, y, &b), "Pixel access failed");
            require(equal(a, b), "Memory/callback pixels differ");
            require(!lossless || equal(original, a), "Lossless roundtrip changed pixels");
        }
    }
    std::printf("%s: %lu bytes, memory/callback pixels match, %u reads, %u seeks\n", name,
                static_cast<unsigned long>(length), stream.reads, stream.seeks);
}

static void exercise(HMODULE module) {
    API api(module);
    api.Initialise(TRUE);
    struct Cleanup { API &api; ~Cleanup() { api.DeInitialise(); } } cleanup{api};
    api.SetOutputMessage(message); // This callback deliberately uses cdecl.
    std::printf("FreeImage version: %s\n", api.GetVersion());
    std::unique_ptr<FIBITMAP, decltype(api.Unload)> source(api.Allocate(64, 48, 24, 0, 0, 0), api.Unload);
    require(source != nullptr, "Bitmap allocation failed");
    for (unsigned y = 0; y < 48; ++y) {
        for (unsigned x = 0; x < 64; ++x) {
            RGBQUAD color{};
            color.rgbRed = static_cast<BYTE>(x * 3 + y);
            color.rgbGreen = static_cast<BYTE>(y * 5 + x);
            color.rgbBlue = static_cast<BYTE>(x + y * 2);
            require(api.SetPixelColor(source.get(), x, y, &color) != FALSE, "Pixel write failed");
        }
    }
    roundtrip(api, source.get(), FIF_PNG, "PNG", PNG_DEFAULT, true);
    roundtrip(api, source.get(), FIF_TIFF, "TIFF", TIFF_DEFLATE, true);
    roundtrip(api, source.get(), FIF_WEBP, "WebP", WEBP_LOSSLESS, true);
    roundtrip(api, source.get(), FIF_JP2, "JP2", JP2_DEFAULT, false);
    roundtrip(api, source.get(), FIF_JXR, "JXR", JXR_LOSSLESS, true);
    roundtrip(api, source.get(), FIF_JPEG, "JPEG", JPEG_QUALITYSUPERB, false);
}

int wmain(int argc, wchar_t **argv) {
    std::setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 2 || argc > 3) return 2;
#if defined(_M_ARM64EC)
    const char *architecture = "ARM64EC";
#elif defined(_M_ARM64)
    const char *architecture = "ARM64";
#elif defined(_M_X64)
    const char *architecture = "x64";
#else
#error Unsupported ARM64X probe architecture
#endif
    USHORT processMachine = 0, nativeMachine = 0;
    if (!IsWow64Process2(GetCurrentProcess(), &processMachine, &nativeMachine) || nativeMachine != IMAGE_FILE_MACHINE_ARM64) {
        std::fprintf(stderr, "This runtime test requires a real Windows ARM64 OS\n");
        return 1;
    }
    std::printf("Consumer: %s; process machine 0x%04x, native machine 0x%04x\n", architecture, processMachine, nativeMachine);
    const bool expectFailure = argc == 3 && std::wcscmp(argv[2], L"--expect-load-failure") == 0;
    HMODULE module = LoadLibraryExW(argv[1], NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (expectFailure) {
        const DWORD error = GetLastError();
        if (module) FreeLibrary(module);
        if (!module && (error == ERROR_BAD_EXE_FORMAT || error == ERROR_EXE_MACHINE_TYPE_MISMATCH)) {
            std::printf("Incompatible unmerged DLL correctly rejected: %lu\n", error);
            return 0;
        }
        std::fprintf(stderr, "Unexpected negative-control result: loaded=%d error=%lu\n", module != NULL, error);
        return 1;
    }
    if (!module) {
        std::fprintf(stderr, "LoadLibraryExW failed: %lu\n", GetLastError());
        return 1;
    }
    int status = 0;
    try {
        exercise(module);
        std::puts("PASS");
    } catch (const std::exception &error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        status = 1;
    }
    FreeLibrary(module);
    return status;
}
