#include "FreeImage.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

#define CHECK(x) do { if(!(x)) { std::fprintf(stderr, "MetadataStripTest line %d: %s\n", __LINE__, #x); std::exit(1); } } while(0)
static const char secret[] = "private-metadata-strip-regression";

static void tag(FIBITMAP *dib, FREE_IMAGE_MDMODEL model, const char *key,
                FREE_IMAGE_MDTYPE type, unsigned count, unsigned length, const void *value) {
    FITAG *t = FreeImage_CreateTag(); CHECK(t);
    CHECK(FreeImage_SetTagKey(t, key)); CHECK(FreeImage_SetTagType(t, type));
    CHECK(FreeImage_SetTagCount(t, count)); CHECK(FreeImage_SetTagLength(t, length));
    CHECK(FreeImage_SetTagValue(t, value)); CHECK(FreeImage_SetMetadata(model, dib, key, t));
    FreeImage_DeleteTag(t);
}
static void text(FIBITMAP *dib, FREE_IMAGE_MDMODEL model, const char *key) {
    tag(dib, model, key, FIDT_ASCII, sizeof(secret), sizeof(secret), secret);
}
static void little(std::vector<BYTE> &v, unsigned value, unsigned bytes) {
    while(bytes--) { v.push_back(static_cast<BYTE>(value)); value >>= 8; }
}
static std::vector<BYTE> exif() {
    std::vector<BYTE> v = {'E','x','i','f',0,0,'I','I',42,0,8,0,0,0};
    little(v, 2, 2);
    little(v, 0x010e, 2); little(v, 2, 2); little(v, sizeof(secret), 4); little(v, 38, 4);
    little(v, 0x0112, 2); little(v, 3, 2); little(v, 1, 4); little(v, 6, 4);
    little(v, 0, 4);
    v.insert(v.end(), secret, secret + sizeof(secret));
    return v;
}
static FIBITMAP *image() {
    FIBITMAP *dib = FreeImage_Allocate(64, 32, 24); CHECK(dib);
    for(unsigned y = 0; y < 32; ++y) for(unsigned x = 0; x < 64 * 3; ++x)
        FreeImage_GetScanLine(dib, y)[x] = static_cast<BYTE>(x + y * 3);
    FreeImage_SetDotsPerMeterX(dib, 3780); FreeImage_SetDotsPerMeterY(dib, 3780);
    text(dib, FIMD_COMMENTS, "Comment"); text(dib, FIMD_EXIF_MAIN, "ImageDescription");
    text(dib, FIMD_IPTC, "Caption-Abstract"); text(dib, FIMD_XMP, "XMLPacket");
    text(dib, FIMD_CUSTOM, "private");
    const WORD orientation = 6;
    tag(dib, FIMD_EXIF_MAIN, "Orientation", FIDT_SHORT, 1, 2, &orientation);
    const std::vector<BYTE> raw = exif();
    tag(dib, FIMD_EXIF_RAW, "ExifRaw", FIDT_BYTE, unsigned(raw.size()), unsigned(raw.size()), raw.data());
    // Minimal ICC header accepted by libpng's header/tag-table validation.
    BYTE icc[512] = {}; icc[2] = 2; icc[8] = 4; icc[9] = 0x30;
    icc[131] = 1; std::memcpy(icc + 132, "cprt", 4); icc[139] = 144;
    icc[142] = 1; icc[143] = 112; std::memcpy(icc + 144, "text", 4);
    unsigned random = 17;
    for(unsigned i = 152; i < sizeof(icc) - 1; ++i) {
        random = random * 1664525u + 1013904223u;
        icc[i] = static_cast<BYTE>(32 + ((random >> 24) % 95));
    }
    std::memcpy(icc + 12, "mntrRGB XYZ ", 12); std::memcpy(icc + 36, "acsp", 4);
    const BYTE d50[] = {0,0,0xf6,0xd6,0,1,0,0,0,0,0xd3,0x2d};
    std::memcpy(icc + 68, d50, sizeof(d50));
    CHECK(FreeImage_CreateICCProfile(dib, icc, sizeof(icc))->data);
    FIBITMAP *thumb = FreeImage_Allocate(8, 4, 24); CHECK(thumb);
    CHECK(FreeImage_SetThumbnail(dib, thumb)); FreeImage_Unload(thumb);
    return dib;
}
static bool clean(FIBITMAP *dib) {
    if(!dib || FreeImage_GetICCProfile(dib)->size || FreeImage_GetThumbnail(dib)) return false;
    for(int model = FIMD_COMMENTS; model <= FIMD_EXIF_RAW; ++model)
        if(model != FIMD_ANIMATION && FreeImage_GetMetadataCount(static_cast<FREE_IMAGE_MDMODEL>(model), dib)) return false;
    return true;
}
static void original(FIBITMAP *dib) {
    CHECK(FreeImage_GetICCProfile(dib)->size == 512); CHECK(FreeImage_GetThumbnail(dib));
    CHECK(FreeImage_GetMetadataCount(FIMD_COMMENTS, dib) == 1);
    CHECK(FreeImage_GetMetadataCount(FIMD_EXIF_RAW, dib) == 1);
    CHECK(FreeImage_GetMetadataCount(FIMD_CUSTOM, dib) == 1);
}
static FIBITMAP *load(FREE_IMAGE_FORMAT fif, FIMEMORY *memory, int flags) {
    CHECK(FreeImage_SeekMemory(memory, 0, SEEK_SET));
    FIBITMAP *dib = FreeImage_LoadFromMemory(fif, memory, flags); CHECK(dib); return dib;
}
static void roundTrip(FREE_IMAGE_FORMAT fif, int flags) {
    if(!FreeImage_FIFSupportsWriting(fif)) return;
    std::printf("Checking %s\n", FreeImage_GetFormatFromFIF(fif)); std::fflush(stdout);
    FIBITMAP *src = image();
    FIMEMORY *memory = FreeImage_OpenMemory(); CHECK(memory);
    CHECK(FreeImage_SaveToMemory(fif, src, memory, flags));
    FIBITMAP *plain = load(fif, memory, 0);
    CHECK(FreeImage_GetICCProfile(plain)->size == 512); CHECK(!clean(plain));
    FIBITMAP *stripped = load(fif, memory, FREEIMAGE_STRIP);
    CHECK(clean(stripped)); CHECK(FreeImage_GetWidth(stripped) == 64 && FreeImage_GetHeight(stripped) == 32);
    CHECK(FreeImage_GetBPP(plain) == FreeImage_GetBPP(stripped));
    for(unsigned y = 0; y < 32; ++y)
        CHECK(!std::memcmp(FreeImage_GetScanLine(plain, y), FreeImage_GetScanLine(stripped, y), FreeImage_GetLine(plain)));
    FreeImage_Unload(stripped); FreeImage_Unload(plain);
    if(FreeImage_FIFSupportsNoPixels(fif)) {
        stripped = load(fif, memory, FREEIMAGE_STRIP | FIF_LOAD_NOPIXELS);
        CHECK(clean(stripped) && !FreeImage_HasPixels(stripped)); FreeImage_Unload(stripped);
    }
    if(fif == FIF_JPEG) {
        stripped = load(fif, memory, FREEIMAGE_STRIP | (16 << 16));
        CHECK(clean(stripped) && FreeImage_GetWidth(stripped) == 16 && FreeImage_GetHeight(stripped) == 8);
        FreeImage_Unload(stripped);
        stripped = load(fif, memory, FREEIMAGE_STRIP | JPEG_EXIFROTATE);
        CHECK(clean(stripped) && FreeImage_GetWidth(stripped) == 32 && FreeImage_GetHeight(stripped) == 64);
        FreeImage_Unload(stripped);
    }
    FreeImage_CloseMemory(memory);
    memory = FreeImage_OpenMemory(); CHECK(memory);
    CHECK(FreeImage_SaveToMemory(fif, src, memory, flags | FREEIMAGE_STRIP)); original(src);
    stripped = load(fif, memory, 0);
    CHECK(FreeImage_GetICCProfile(stripped)->size == 0); CHECK(!FreeImage_GetThumbnail(stripped));
    CHECK(!FreeImage_GetMetadataCount(FIMD_XMP, stripped)); CHECK(!FreeImage_GetMetadataCount(FIMD_IPTC, stripped));
    FITAG *description = NULL;
    CHECK(!FreeImage_GetMetadata(FIMD_EXIF_MAIN, stripped, "ImageDescription", &description));
    CHECK(!FreeImage_GetMetadataCount(FIMD_EXIF_RAW, stripped));
    FreeImage_Unload(stripped); FreeImage_CloseMemory(memory); FreeImage_Unload(src);
}
static void inPlaceAndAnimation() {
    CHECK(!FreeImage_StripMetadata(NULL));
    FIBITMAP *src = image(); const BYTE *pixels = FreeImage_GetBits(src);
    const DWORD time = 120, loop = 3; const BYTE disposal = 2;
    tag(src, FIMD_ANIMATION, "FrameTime", FIDT_LONG, 1, 4, &time);
    tag(src, FIMD_ANIMATION, "Loop", FIDT_LONG, 1, 4, &loop);
    tag(src, FIMD_ANIMATION, "DisposalMethod", FIDT_BYTE, 1, 1, &disposal);
    FreeImage_GetICCProfile(src)->flags |= FIICC_COLOR_IS_CMYK;
    CHECK(FreeImage_StripMetadata(src)); CHECK(clean(src));
    CHECK(FreeImage_GetICCProfile(src)->flags & FIICC_COLOR_IS_CMYK);
    CHECK(pixels == FreeImage_GetBits(src)); CHECK(FreeImage_GetDotsPerMeterX(src) == 3780);
    CHECK(FreeImage_GetMetadataCount(FIMD_ANIMATION, src) == 3);
    CHECK(FreeImage_StripMetadata(src)); FreeImage_Unload(src);

    src = FreeImage_Allocate(17, 9, 8); CHECK(src);
    for(unsigned i = 0; i < 256; ++i) {
        RGBQUAD *color = FreeImage_GetPalette(src) + i;
        color->rgbRed = static_cast<BYTE>(i); color->rgbGreen = 40; color->rgbBlue = 80;
    }
    BYTE alpha[] = {0,255}; FreeImage_SetTransparencyTable(src, alpha, 2);
    text(src, FIMD_COMMENTS, "Comment");
    tag(src, FIMD_ANIMATION, "FrameTime", FIDT_LONG, 1, 4, &time);
    tag(src, FIMD_ANIMATION, "Loop", FIDT_LONG, 1, 4, &loop);
    tag(src, FIMD_ANIMATION, "DisposalMethod", FIDT_BYTE, 1, 1, &disposal);
    FIMEMORY *memory = FreeImage_OpenMemory(); CHECK(memory);
    CHECK(FreeImage_SaveToMemory(FIF_GIF, src, memory, FREEIMAGE_STRIP));
    CHECK(FreeImage_GetMetadataCount(FIMD_COMMENTS, src));
    FIBITMAP *decoded = load(FIF_GIF, memory, FREEIMAGE_STRIP); CHECK(clean(decoded));
    CHECK(FreeImage_IsTransparent(decoded));
    FITAG *t = NULL; CHECK(FreeImage_GetMetadata(FIMD_ANIMATION, decoded, "FrameTime", &t));
    CHECK(*static_cast<const DWORD *>(FreeImage_GetTagValue(t)) == time);
    CHECK(FreeImage_GetPalette(decoded)[1].rgbGreen == 40);
    FreeImage_Unload(decoded); FreeImage_CloseMemory(memory); FreeImage_Unload(src);
}
static FIBITMAP *saveSource;
static int expectedFlags;
static FIBITMAP *DLL_CALLCONV pluginLoad(FreeImageIO *, fi_handle, int, int flags, void *) {
    CHECK(flags == expectedFlags); return image();
}
static BOOL DLL_CALLCONV pluginSave(FreeImageIO *, FIBITMAP *dib, fi_handle, int, int flags, void *) {
    CHECK(flags == expectedFlags); CHECK(clean(dib)); original(saveSource);
    CHECK(dib != saveSource && FreeImage_GetBits(dib) == FreeImage_GetBits(saveSource));
    CHECK(FreeImage_GetICCProfile(dib)->flags == FreeImage_GetICCProfile(saveSource)->flags);
    return FALSE; // Exercise cleanup on an encoder failure, too.
}
static void DLL_CALLCONV initPlugin(Plugin *plugin, int) {
    plugin->load_proc = pluginLoad; plugin->save_proc = pluginSave;
}
static void pluginFlagsAndFailure() {
    FREE_IMAGE_FORMAT fif = FreeImage_RegisterLocalPlugin(initPlugin, "STRIPTEST", "Strip flag test", "striptest");
    CHECK(fif != FIF_UNKNOWN); expectedFlags = (16 << 16) | JPEG_ACCURATE;
    FIMEMORY *memory = FreeImage_OpenMemory(); CHECK(memory);
    FIBITMAP *dib = load(fif, memory, expectedFlags | FREEIMAGE_STRIP); CHECK(clean(dib)); FreeImage_Unload(dib);
    saveSource = image(); FreeImage_GetICCProfile(saveSource)->flags |= FIICC_COLOR_IS_CMYK;
    CHECK(!FreeImage_SaveToMemory(fif, saveSource, memory, expectedFlags | FREEIMAGE_STRIP)); original(saveSource);
    FreeImage_Unload(saveSource);
    dib = load(fif, memory, expectedFlags); original(dib); FreeImage_Unload(dib);
    FreeImage_CloseMemory(memory);
}
static void threadedLoads() {
    if(!FreeImage_FIFSupportsWriting(FIF_PNG)) return;
    FIBITMAP *src = image(); FIMEMORY *memory = FreeImage_OpenMemory(); CHECK(memory);
    CHECK(FreeImage_SaveToMemory(FIF_PNG, src, memory));
    BYTE *bytes = NULL; DWORD size = 0; CHECK(FreeImage_AcquireMemory(memory, &bytes, &size));
    std::atomic<bool> success(true);
    auto worker = [&](bool strip) {
        for(int i = 0; i < 20; ++i) {
            FIMEMORY *input = FreeImage_OpenMemory(bytes, size);
            FIBITMAP *decoded = FreeImage_LoadFromMemory(FIF_PNG, input, strip ? FREEIMAGE_STRIP : 0);
            if(!decoded || clean(decoded) != strip) success = false;
            FreeImage_Unload(decoded); FreeImage_CloseMemory(input);
        }
    };
    std::thread a(worker, true), b(worker, false); a.join(); b.join(); CHECK(success);
    FreeImage_CloseMemory(memory); FreeImage_Unload(src);
}
static void multiPage() {
    if(!FreeImage_FIFSupportsWriting(FIF_TIFF)) return;
    const char *path = "metadata-strip-multipage.tif";
    FIBITMAP *src = image();
    FIMULTIBITMAP *multi = FreeImage_OpenMultiBitmap(FIF_TIFF, path, TRUE, FALSE, TRUE); CHECK(multi);
    FreeImage_AppendPage(multi, src); FreeImage_AppendPage(multi, src);
    CHECK(FreeImage_CloseMultiBitmap(multi));
    multi = FreeImage_OpenMultiBitmap(FIF_TIFF, path, FALSE, TRUE, TRUE, FREEIMAGE_STRIP); CHECK(multi);
    CHECK(FreeImage_GetPageCount(multi) == 2);
    for(int i = 0; i < 2; ++i) {
        FIBITMAP *page = FreeImage_LockPage(multi, i); CHECK(clean(page));
        FreeImage_UnlockPage(multi, page, FALSE);
    }
    CHECK(FreeImage_CloseMultiBitmap(multi));
    multi = FreeImage_OpenMultiBitmap(FIF_TIFF, path, FALSE, TRUE, TRUE); CHECK(multi);
    FIMEMORY *memory = FreeImage_OpenMemory(); CHECK(memory);
    CHECK(FreeImage_SaveMultiBitmapToMemory(FIF_TIFF, multi, memory, FREEIMAGE_STRIP));
    FIBITMAP *page = FreeImage_LockPage(multi, 0); CHECK(page && !clean(page));
    FreeImage_UnlockPage(multi, page, FALSE); CHECK(FreeImage_CloseMultiBitmap(multi));
    CHECK(FreeImage_SeekMemory(memory, 0, SEEK_SET));
    multi = FreeImage_LoadMultiBitmapFromMemory(FIF_TIFF, memory, FREEIMAGE_STRIP); CHECK(multi);
    CHECK(FreeImage_GetPageCount(multi) == 2);
    page = FreeImage_LockPage(multi, 1); CHECK(clean(page));
    FreeImage_UnlockPage(multi, page, FALSE); CHECK(FreeImage_CloseMultiBitmap(multi));
    FreeImage_CloseMemory(memory);
    // New/modified cached pages take a separate multipage save path.
    multi = FreeImage_OpenMultiBitmap(FIF_TIFF, path, TRUE, FALSE, TRUE); CHECK(multi);
    FreeImage_AppendPage(multi, src); CHECK(FreeImage_CloseMultiBitmap(multi, FREEIMAGE_STRIP));
    page = FreeImage_Load64(FIF_TIFF, path, FREEIMAGE_STRIP); CHECK(clean(page)); FreeImage_Unload(page);
    page = FreeImage_Load(FIF_TIFF, path);
    CHECK(page && !FreeImage_GetICCProfile(page)->size && !FreeImage_GetMetadataCount(FIMD_XMP, page));
    FreeImage_Unload(page); FreeImage_Unload(src); CHECK(!std::remove(path));
}
// A six-byte UNDEFINED IPTC field must stay six bytes, not six LONGs.
// The old TIFF bridge passed four times the allocation size to the IPTC reader.
static void minimalTiffIptc() {
    if(!FreeImage_FIFSupportsReading(FIF_TIFF)) return;
    static BYTE fixture[] = {
        0x49, 0x49, 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01,
        0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x01,
        0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01,
        0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x15, 0x01,
        0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x16, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17, 0x01,
        0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xbb, 0x83,
        0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x1c, 0x02, 0x78, 0x00, 0x01, 0x78, 0x40,
    };
    FIMEMORY *memory = FreeImage_OpenMemory(fixture, sizeof(fixture)); CHECK(memory);
    FIBITMAP *plain = load(FIF_TIFF, memory, 0);
    FITAG *caption = NULL;
    CHECK(FreeImage_GetMetadata(FIMD_IPTC, plain, "Caption-Abstract", &caption));
    CHECK(!std::strcmp(static_cast<const char *>(FreeImage_GetTagValue(caption)), "x"));
    FIBITMAP *stripped = load(FIF_TIFF, memory, FREEIMAGE_STRIP);
    CHECK(clean(stripped));
    CHECK(FreeImage_GetWidth(stripped) == 1 && FreeImage_GetHeight(stripped) == 1);
    CHECK(FreeImage_GetScanLine(plain, 0)[0] == FreeImage_GetScanLine(stripped, 0)[0]);
    FreeImage_Unload(stripped); FreeImage_Unload(plain); FreeImage_CloseMemory(memory);
}

int main() {
    static_assert((FREEIMAGE_STRIP & (16 << 16)) == 0, "Keep JPEG scaled-load flags available");
    FreeImage_Initialise();
    roundTrip(FIF_JPEG, JPEG_QUALITYSUPERB); roundTrip(FIF_PNG, 0);
    roundTrip(FIF_TIFF, 0); roundTrip(FIF_WEBP, WEBP_LOSSLESS); roundTrip(FIF_JXR, JXR_LOSSLESS);
    inPlaceAndAnimation(); pluginFlagsAndFailure(); threadedLoads(); multiPage(); minimalTiffIptc();
    FreeImage_DeInitialise();
    std::puts("Metadata stripping, source preservation, JPEG flags, animation, threads and multipage checks passed");
}
