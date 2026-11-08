#include "../Examples/codec-diagnostics/Preview.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>

namespace {
struct BitmapDeleter { void operator()(FIBITMAP *p) const { FreeImage_Unload(p); } };
struct MemoryDeleter { void operator()(FIMEMORY *p) const { FreeImage_CloseMemory(p); } };
typedef std::unique_ptr<FIBITMAP, BitmapDeleter> Bitmap;
typedef std::unique_ptr<FIMEMORY, MemoryDeleter> Memory;
void Require(bool ok, const char *what) {
    if(!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
void CheckPreview(FIBITMAP *source) {
    const FREE_IMAGE_TYPE type = FreeImage_GetImageType(source);
    Bitmap copy(FreeImage_Clone(source));
    Bitmap preview(codec_example::MakeRGBPreview(source));
    Require(copy && preview, "create typed-image preview");
    Require(FreeImage_GetImageType(preview.get()) == FIT_BITMAP && FreeImage_GetBPP(preview.get()) == 24, "preview is 24-bit bitmap");
    Require(FreeImage_GetWidth(preview.get()) == FreeImage_GetWidth(source) &&
        FreeImage_GetHeight(preview.get()) == FreeImage_GetHeight(source), "preview dimensions");
    Require(FreeImage_GetImageType(source) == type, "preserve native type");
    for(unsigned y = 0; y < FreeImage_GetHeight(source); ++y)
        Require(!std::memcmp(FreeImage_GetScanLine(source, y), FreeImage_GetScanLine(copy.get(), y),
            FreeImage_GetLine(source)), "preserve native pixels including alpha");
}
Bitmap RGB16() {
    Bitmap source(FreeImage_AllocateT(FIT_RGB16, 64, 48));
    Require(bool(source), "allocate RGB16 fixture");
    for(unsigned y = 0; y < 48; ++y) {
        FIRGB16 *row = reinterpret_cast<FIRGB16 *>(FreeImage_GetScanLine(source.get(), y));
        for(unsigned x = 0; x < 64; ++x) {
            row[x].red = static_cast<WORD>(x * 1024);
            row[x].green = static_cast<WORD>(y * 1024);
            row[x].blue = 32768;
        }
    }
    return source;
}
Bitmap RGBAF() {
    Bitmap source(FreeImage_AllocateT(FIT_RGBAF, 64, 48));
    Require(bool(source), "allocate HDR fixture");
    for(unsigned y = 0; y < 48; ++y) {
        FIRGBAF *row = reinterpret_cast<FIRGBAF *>(FreeImage_GetScanLine(source.get(), y));
        for(unsigned x = 0; x < 64; ++x) {
            row[x].red = 0.25f + float(x) / 8;
            row[x].green = 0.25f + float(y) / 16;
            row[x].blue = 0.5f;
            row[x].alpha = (x & 1) ? 0.5f : 1.0f;
        }
    }
    return source;
}
Memory Encode(FREE_IMAGE_FORMAT format, FIBITMAP *source, int flags) {
    Memory stream(FreeImage_OpenMemory());
    Require(stream && FreeImage_SaveToMemory(format, source, stream.get(), flags), "encode synthetic codec fixture");
    Require(FreeImage_SeekMemory(stream.get(), 0, SEEK_SET), "rewind encoded fixture");
    return stream;
}
void CheckDecode(FREE_IMAGE_FORMAT format, FREE_IMAGE_TYPE expected, Memory &stream) {
    Require(FreeImage_GetFileTypeFromMemory(stream.get()) == format, "content-based memory detection");
    Require(FreeImage_TellMemory(stream.get()) == 0, "detection preserves memory position");
    Bitmap decoded(FreeImage_LoadFromMemory(format, stream.get()));
    Require(decoded && FreeImage_HasPixels(decoded.get()), "decode synthetic fixture");
    Require(FreeImage_GetImageType(decoded.get()) == expected, "preserve codec native type");
    CheckPreview(decoded.get());
}
}

int main(int argc, char **argv) {
    Require(argc == 2, "temporary fixture filename argument");
    FreeImage_Initialise();
    {
        Bitmap rgb = RGB16();
        Bitmap hdr = RGBAF();
        CheckPreview(rgb.get());
        CheckPreview(hdr.get());
#if FREEIMAGE_TEST_JP2
        Memory jp2 = Encode(FIF_JP2, rgb.get(), 1);
        CheckDecode(FIF_JP2, FIT_RGB16, jp2);
        Memory j2k = Encode(FIF_J2K, rgb.get(), 1);
        BYTE *data = NULL; DWORD size = 0;
        Require(FreeImage_AcquireMemory(j2k.get(), &data, &size) && size, "raw codestream bytes");
        {
            std::ofstream file(argv[1], std::ios::binary);
            Require(bool(file.write(reinterpret_cast<const char *>(data), size)), "write mislabeled codestream");
        }
        Require(FreeImage_GetFIFFromFilename(argv[1]) == FIF_JP2, "fixture has JP2 extension");
        Require(FreeImage_GetFileType(argv[1]) == FIF_J2K, "signature overrides wrong extension");
        Bitmap detected(FreeImage_Load(FreeImage_GetFileType(argv[1]), argv[1]));
        Require(detected && FreeImage_HasPixels(detected.get()), "content-selected load succeeds");
        CheckPreview(detected.get());
        Require(std::remove(argv[1]) == 0, "remove temporary codestream");
#endif
#if FREEIMAGE_TEST_JXR
        Memory jxr = Encode(FIF_JXR, hdr.get(), JXR_LOSSLESS);
        CheckDecode(FIF_JXR, FIT_RGBAF, jxr);
#endif
    }
    FreeImage_DeInitialise();
    std::puts("Native codec types, content detection and explicit RGB previews passed");
    return 0;
}
