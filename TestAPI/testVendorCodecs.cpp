// Integration tests for the bundled OpenEXR and TIFF codecs.
// Explicit checks keep this coverage active in Release builds.

#include "FreeImage.h"
#include "ImfChannelList.h"
#include "ImfCompression.h"
#include "ImfDeepFrameBuffer.h"
#include "ImfDeepScanLineInputFile.h"
#include "ImfDeepScanLineOutputFile.h"
#include "ImfHeader.h"
#include "ImfPartType.h"
#include "ImfRgbaFile.h"
#include "ojph_arch.h"
#include "tiffio.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
static void check(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
static void checkSimdConfiguration() {
    const int level = ojph::get_cpu_ext_level();
#if FREEIMAGE_TEST_OPENJPH_SIMD && !defined(_M_ARM64EC) &&                                         \
    (defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64))
    // x86_64 has SSE2 as its baseline. This also catches accidentally keeping
    // OJPH_DISABLE_SIMD in the library while the build reports SIMD enabled.
#if defined(__x86_64__) || (defined(_M_X64) && !defined(_M_ARM64EC))
    check(level >= ojph::X86_CPU_EXT_LEVEL_SSE2, "OpenJPH SIMD dispatch was not enabled");
#endif
    printf("OpenJPH SIMD enabled; runtime CPU level %d\n", level);
#else
    check(level == 0, "OpenJPH portable build unexpectedly enabled SIMD");
    puts("OpenJPH portable implementation enabled");
#endif
}
static void exrRoundtrip(Imf::Compression codec) {
    const int w = 37, h = 65;
    std::vector<Imf::Rgba> pixels(w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            auto &p = pixels[y * w + x];
            p.r = float(x % 16) / 16;
            p.g = float(y % 16) / 16;
            p.b = float((x + y) % 16) / 16;
            p.a = 1;
        }
    const char *path = "vendor-codec.exr";
    {
        Imf::Header header(w, h);
        header.compression() = codec;
        Imf::RgbaOutputFile out(path, header, Imf::WRITE_RGBA);
        out.setFrameBuffer(pixels.data(), 1, w);
        out.writePixels(h);
    }
    FIBITMAP *image = FreeImage_Load(FIF_EXR, path);
    check(image && FreeImage_GetWidth(image) == w && FreeImage_GetHeight(image) == h,
          "EXR load failed");
    check(FreeImage_GetImageType(image) == FIT_RGBAF, "EXR type mismatch");
    for (int y = 0; y < h; ++y) {
        auto *row = reinterpret_cast<FIRGBAF *>(FreeImage_GetScanLine(image, h - 1 - y));
        for (int x = 0; x < w; ++x) {
            const auto &p = pixels[y * w + x];
            if (codec == Imf::LJ2K_COMPRESSION)
                check(std::isfinite(row[x].red) && std::fabs(row[x].red - float(p.r)) < 0.1f &&
                          std::isfinite(row[x].green) &&
                          std::fabs(row[x].green - float(p.g)) < 0.1f &&
                          std::isfinite(row[x].blue) &&
                          std::fabs(row[x].blue - float(p.b)) < 0.1f && row[x].alpha == 1,
                      "Lossy EXR mismatch");
            else
                check(row[x].red == float(p.r) && row[x].green == float(p.g) &&
                          row[x].blue == float(p.b) && row[x].alpha == 1,
                      "EXR pixels differ");
        }
    }
    FreeImage_Unload(image);
    std::remove(path);
    std::string name;
    Imf::getCompressionNameFromId(codec, name);
    printf("EXR %s passed\n", name.c_str());
}
static void deepZstd() {
    const int w = 7, h = 33, n = w * h;
    std::vector<unsigned> counts(n), readCounts(n);
    std::vector<float> samples(n * 3), readSamples(n * 3, -1);
    std::vector<float *> pointers(n), readPointers(n);
    for (int i = 0; i < n; ++i) {
        counts[i] = unsigned(i % 4);
        pointers[i] = &samples[i * 3];
        readPointers[i] = &readSamples[i * 3];
        for (unsigned s = 0; s < counts[i]; ++s)
            samples[i * 3 + s] = float(i + s) / 16;
    }
    Imf::Header hdr(w, h);
    hdr.setType(Imf::DEEPSCANLINE);
    hdr.compression() = Imf::ZSTD_COMPRESSION;
    hdr.channels().insert("Z", Imf::Channel(Imf::FLOAT));
    const char *path = "vendor-deep.exr";
    Imf::DeepFrameBuffer fb;
    fb.insertSampleCountSlice(Imf::Slice(Imf::UINT, reinterpret_cast<char *>(counts.data()),
                                         sizeof(unsigned), w * sizeof(unsigned)));
    fb.insert("Z", Imf::DeepSlice(Imf::FLOAT, reinterpret_cast<char *>(pointers.data()),
                                  sizeof(float *), w * sizeof(float *), sizeof(float)));
    {
        Imf::DeepScanLineOutputFile out(path, hdr);
        out.setFrameBuffer(fb);
        out.writePixels(h);
    }
    Imf::DeepFrameBuffer rf;
    rf.insertSampleCountSlice(Imf::Slice(Imf::UINT, reinterpret_cast<char *>(readCounts.data()),
                                         sizeof(unsigned), w * sizeof(unsigned)));
    rf.insert("Z", Imf::DeepSlice(Imf::FLOAT, reinterpret_cast<char *>(readPointers.data()),
                                  sizeof(float *), w * sizeof(float *), sizeof(float)));
    {
        Imf::DeepScanLineInputFile in(path);
        in.setFrameBuffer(rf);
        in.readPixelSampleCounts(0, h - 1);
        in.readPixels(0, h - 1);
        check(counts == readCounts, "Deep sample count mismatch");
        for (int i = 0; i < n; ++i)
            for (unsigned s = 0; s < counts[i]; ++s)
                check(samples[i * 3 + s] == readSamples[i * 3 + s], "Deep ZSTD value mismatch");
    }
    std::remove(path);
    puts("Deep EXR ZSTD with variable sample counts passed");
}
static void tiffRoundtrip(int flag) {
    const int w = 61, h = 35;
    FIBITMAP *image = FreeImage_Allocate(w, h, 24);
    check(image, "TIFF allocation failed");
    for (int y = 0; y < h; ++y) {
        auto *row = FreeImage_GetScanLine(image, y);
        for (int x = 0; x < w * 3; ++x)
            row[x] = static_cast<BYTE>(x + y);
    }
    FIMEMORY *mem = FreeImage_OpenMemory();
    check(FreeImage_SaveToMemory(FIF_TIFF, image, mem, flag), "TIFF save failed");
    FreeImage_SeekMemory(mem, 0, SEEK_SET);
    FIBITMAP *loaded = FreeImage_LoadFromMemory(FIF_TIFF, mem);
    check(loaded && FreeImage_GetWidth(loaded) == w && FreeImage_GetHeight(loaded) == h,
          "TIFF load failed");
    for (int y = 0; y < h; ++y)
        check(
            !std::memcmp(FreeImage_GetScanLine(image, y), FreeImage_GetScanLine(loaded, y), w * 3),
            "TIFF pixels differ");
    FreeImage_Unload(loaded);
    FreeImage_CloseMemory(mem);
    FreeImage_Unload(image);
    printf("TIFF compression 0x%x passed\n", flag);
}
static tmsize_t readFile(thandle_t h, void *p, tmsize_t n) {
    return fread(p, 1, n, static_cast<FILE *>(h));
}
static tmsize_t writeFile(thandle_t h, void *p, tmsize_t n) {
    return fwrite(p, 1, n, static_cast<FILE *>(h));
}
static toff_t seekFile(thandle_t h, toff_t n, int whence) {
    if (fseek(static_cast<FILE *>(h), static_cast<long>(n), whence))
        return static_cast<toff_t>(-1);
    return static_cast<toff_t>(ftell(static_cast<FILE *>(h)));
}
static int closeFile(thandle_t h) { return fclose(static_cast<FILE *>(h)); }
static toff_t sizeFile(thandle_t h) {
    FILE *f = static_cast<FILE *>(h);
    long old = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, old, SEEK_SET);
    return size;
}
static int mapFile(thandle_t, void **, toff_t *) { return 0; }
static void unmapFile(thandle_t, void *, toff_t) {}
static TIFF *openTiff(const char *path, const char *mode) {
    FILE *f = fopen(path, mode[0] == 'w' ? "wb+" : "rb");
    check(f != nullptr, "TIFF file open failed");
    return TIFFClientOpen(path, mode, f, readFile, writeFile, seekFile, closeFile, sizeFile,
                          mapFile, unmapFile);
}
static void g3Roundtrip() {
    const int w = 1728, h = 8;
    const char *path = "vendor-fax.tif";
    TIFF *tif = openTiff(path, "w");
    check(tif, "Fax TIFF open failed");
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX3);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, h);
    std::vector<unsigned char> row(w / 8, 0x55);
    for (int y = 0; y < h; ++y)
        check(TIFFWriteScanline(tif, row.data(), y) >= 0, "Fax write failed");
    TIFFClose(tif);
    tif = openTiff(path, "r");
    check(tif, "Fax reopen failed");
    std::vector<unsigned char> raw(static_cast<size_t>(TIFFRawStripSize(tif, 0)));
    check(TIFFReadRawStrip(tif, 0, raw.data(), raw.size()) > 0, "Fax strip read failed");
    TIFFClose(tif);
    FIMEMORY *mem = FreeImage_OpenMemory(raw.data(), static_cast<DWORD>(raw.size()));
    FIBITMAP *image = FreeImage_LoadFromMemory(FIF_FAXG3, mem);
    check(image && FreeImage_GetWidth(image) == w && FreeImage_GetHeight(image) >= h,
          "G3 load failed");
    for (int y = 0; y < h; ++y)
        check(!std::memcmp(FreeImage_GetScanLine(image, FreeImage_GetHeight(image) - 1 - y),
                           row.data(), row.size()),
              "G3 pixels differ");
    FreeImage_Unload(image);
    FreeImage_CloseMemory(mem);
    std::remove(path);
    puts("Raw G3 fax decoding passed");
}
int main() {
    try {
        FreeImage_Initialise();
        checkSimdConfiguration();
        exrRoundtrip(Imf::ZIP_COMPRESSION);
        exrRoundtrip(Imf::PIZ_COMPRESSION);
        exrRoundtrip(Imf::ZSTD_COMPRESSION);
        exrRoundtrip(Imf::HTJ2K256_COMPRESSION);
        exrRoundtrip(Imf::HTJ2K32_COMPRESSION);
        exrRoundtrip(Imf::LJ2K_COMPRESSION);
        deepZstd();
        tiffRoundtrip(TIFF_NONE);
        tiffRoundtrip(TIFF_LZW);
        tiffRoundtrip(TIFF_ADOBE_DEFLATE);
        tiffRoundtrip(TIFF_PACKBITS);
        g3Roundtrip();
        FreeImage_DeInitialise();
        return 0;
    } catch (const std::exception &e) {
        fprintf(stderr, "FAIL: %s\n", e.what());
        return 1;
    }
}
