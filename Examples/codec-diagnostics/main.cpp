#include "Preview.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
struct BitmapDeleter { void operator()(FIBITMAP *p) const { FreeImage_Unload(p); } };
struct MemoryDeleter { void operator()(FIMEMORY *p) const { FreeImage_CloseMemory(p); } };
typedef std::unique_ptr<FIBITMAP, BitmapDeleter> Bitmap;
typedef std::unique_ptr<FIMEMORY, MemoryDeleter> Memory;
struct Library {
    Library() { FreeImage_Initialise(); }
    ~Library() { FreeImage_DeInitialise(); }
};
const char *Format(FREE_IMAGE_FORMAT format) {
    const char *name = FreeImage_GetFormatFromFIF(format);
    return name ? name : "UNKNOWN";
}
const char *Type(FIBITMAP *dib) {
    switch(FreeImage_GetImageType(dib)) {
        case FIT_BITMAP: return "FIT_BITMAP";
        case FIT_RGB16: return "FIT_RGB16";
        case FIT_RGBA16: return "FIT_RGBA16";
        case FIT_RGBF: return "FIT_RGBF";
        case FIT_RGBAF: return "FIT_RGBAF";
        default: return "OTHER";
    }
}
void Message(FREE_IMAGE_FORMAT format, const char *message) {
    std::fprintf(stderr, "%s: %s\n", Format(format), message);
}
std::uint64_t Hash(FIBITMAP *dib) {
    std::uint64_t hash = UINT64_C(14695981039346656037);
    for(unsigned y = 0; y < FreeImage_GetHeight(dib); ++y) {
        const BYTE *row = FreeImage_GetScanLine(dib, y);
        for(unsigned x = 0; x < FreeImage_GetLine(dib); ++x) {
            hash ^= row[x]; hash *= UINT64_C(1099511628211);
        }
    }
    return hash;
}
}

int main(int argc, char **argv) {
    const bool memory_check = argc > 1 && std::string(argv[1]) == "--memory";
    const int first = memory_check ? 2 : 1;
    if(argc < first + 1 || argc > first + 2) {
        std::fprintf(stderr, "Usage: CodecDiagnostics [--memory] input [preview.png]\n");
        return 2;
    }
    Library library;
    FreeImage_SetOutputMessage(Message);
    try {
        const char *input = argv[first];
        {
            std::ifstream readable(input, std::ios::binary);
            if(!readable) { std::fprintf(stderr, "input.open=failed\n"); return 3; }
        }
        const FREE_IMAGE_FORMAT signature = FreeImage_GetFileType(input);
        const FREE_IMAGE_FORMAT extension = FreeImage_GetFIFFromFilename(input);
        const FREE_IMAGE_FORMAT format = signature != FIF_UNKNOWN ? signature : extension;
        std::printf("signature=%s extension=%s selected=%s\n", Format(signature), Format(extension), Format(format));
        if(format == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(format)) {
            std::fprintf(stderr, "detection: no enabled reader; check codec build options\n");
            return 3;
        }
        Bitmap decoded(FreeImage_Load(format, input));
        if(!decoded || !FreeImage_HasPixels(decoded.get())) {
            std::fprintf(stderr, "decode.file=failed\n"); return 4;
        }
        const std::uint64_t hash = Hash(decoded.get());
        std::printf("decode.file=success native_type=%s bpp=%u width=%u height=%u\n",
            Type(decoded.get()), FreeImage_GetBPP(decoded.get()), FreeImage_GetWidth(decoded.get()), FreeImage_GetHeight(decoded.get()));
        if(memory_check) {
            std::ifstream file(input, std::ios::binary | std::ios::ate);
            const std::streamoff length = file.tellg();
            if(!file || length <= 0 || static_cast<std::uint64_t>(length) > std::numeric_limits<DWORD>::max() ||
               static_cast<std::uint64_t>(length) > std::numeric_limits<std::size_t>::max() ||
               length > std::numeric_limits<std::streamsize>::max())
                throw std::runtime_error("input does not fit the existing memory-I/O API");
            std::vector<BYTE> bytes(static_cast<std::size_t>(length));
            file.seekg(0);
            if(!file.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(length)))
                throw std::runtime_error("could not read input for memory verification");
            Memory stream(FreeImage_OpenMemory(bytes.data(), static_cast<DWORD>(bytes.size())));
            if(!stream) throw std::runtime_error("could not open memory stream");
            const FREE_IMAGE_FORMAT memory_format = FreeImage_GetFileTypeFromMemory(stream.get());
            const long position = FreeImage_TellMemory(stream.get());
            Bitmap from_memory(FreeImage_LoadFromMemory(memory_format != FIF_UNKNOWN ? memory_format : format, stream.get()));
            const bool same = from_memory && FreeImage_GetImageType(from_memory.get()) == FreeImage_GetImageType(decoded.get()) &&
                FreeImage_GetWidth(from_memory.get()) == FreeImage_GetWidth(decoded.get()) &&
                FreeImage_GetHeight(from_memory.get()) == FreeImage_GetHeight(decoded.get()) &&
                FreeImage_GetBPP(from_memory.get()) == FreeImage_GetBPP(decoded.get()) && Hash(from_memory.get()) == hash;
            std::printf("decode.memory=%s position_after_detection=%ld pixels_match=%s\n",
                from_memory ? "success" : "failed", position, same ? "true" : "false");
            if(!same || position != 0) return 5;
        }
        if(argc == first + 2) {
            Bitmap preview(codec_example::MakeRGBPreview(decoded.get()));
            if(!preview) { std::fprintf(stderr, "preview.conversion=failed; native decode succeeded\n"); return 6; }
            if(!FreeImage_Save(FIF_PNG, preview.get(), argv[first + 1])) {
                std::fprintf(stderr, "preview.save=failed; native decode succeeded\n"); return 7;
            }
            std::printf("preview=success type=FIT_BITMAP bpp=24 policy=%s alpha=discarded\n",
                FreeImage_GetImageType(decoded.get()) == FIT_RGBF || FreeImage_GetImageType(decoded.get()) == FIT_RGBAF ? "Drago-HDR-tone-map" : "8-bit-RGB");
            if(Hash(decoded.get()) != hash) throw std::runtime_error("preview changed native pixels");
        }
    } catch(const std::exception &error) {
        std::fprintf(stderr, "diagnostic: %s\n", error.what()); return 8;
    }
    return 0;
}
