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
static bool encode = false;
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
    if(!std::strcmp(filename,"metadata.heic")) {
        FITAG *tag=NULL;
        Require(FreeImage_GetMetadata(FIMD_EXIF_MAIN,dib,"Orientation",&tag),"fixture EXIF");
        FIBITMAP *thumbnail=FreeImage_GetThumbnail(dib);
        Require(thumbnail && FreeImage_GetWidth(thumbnail)<=16 && FreeImage_GetHeight(thumbnail)<=16,"embedded thumbnail");
        std::vector<BYTE> invalid(bytes); size_t offset=0;
        for(size_t i=4;i+8<invalid.size();++i) if(invalid[i]=='I' && invalid[i+1]=='I' && invalid[i+2]==42 && invalid[i+3]==0 && invalid[i+4]==8) { offset=i; break; }
        Require(offset>4,"locate TIFF metadata");
        for(size_t i=offset-4;i<offset;++i) invalid[i]=255;
        FIMEMORY *malformed=FreeImage_OpenMemory(invalid.data(),static_cast<DWORD>(invalid.size()));
        FIBITMAP *without_exif=FreeImage_LoadFromMemory(format,malformed);
        Require(without_exif && !FreeImage_GetMetadata(FIMD_EXIF_RAW,without_exif,"ExifRaw",&tag),"invalid EXIF offset ignored");
        FreeImage_Unload(without_exif); FreeImage_CloseMemory(malformed);
    }
    if(encode && FreeImage_FIFSupportsWriting(format)) {
        const BYTE exif[] = {'E','x','i','f',0,0,'I','I',42,0,8,0,0,0,1,0,0x12,1,3,0,1,0,0,0,1,0,0,0,0,0,0,0};
        FITAG *tag = FreeImage_CreateTag();
        Require(tag && FreeImage_SetTagKey(tag,"ExifRaw") && FreeImage_SetTagType(tag,FIDT_BYTE) &&
            FreeImage_SetTagCount(tag,sizeof(exif)) && FreeImage_SetTagLength(tag,sizeof(exif)) && FreeImage_SetTagValue(tag,exif), "EXIF input");
        Require(FreeImage_SetMetadata(FIMD_EXIF_RAW,dib,"ExifRaw",tag), "attach EXIF"); FreeImage_DeleteTag(tag);
        FIMEMORY *encoded = FreeImage_OpenMemory();
        Require(FreeImage_SaveToMemory(format, dib, encoded, 100), "encode");
        FreeImage_SeekMemory(encoded, 0, SEEK_SET);
        FIBITMAP *roundtrip = FreeImage_LoadFromMemory(format, encoded);
        Require(roundtrip && FreeImage_GetWidth(roundtrip) == 32 && FreeImage_GetBPP(roundtrip) == (alpha ? 32U : 24U), "roundtrip");
        const BYTE *pixel = FreeImage_GetScanLine(roundtrip, 20) + 4 * (alpha ? 4 : 3);
        Require(std::abs(int(pixel[FI_RGBA_RED]) - 220) < 40, "roundtrip colour");
        if(alpha) Require(pixel[FI_RGBA_ALPHA] > 250, "roundtrip alpha");
        FIICCPROFILE *profile = FreeImage_GetICCProfile(roundtrip);
        Require(profile && profile->size == 128, "roundtrip ICC");
        FITAG *raw = NULL;
        Require(FreeImage_GetMetadata(FIMD_EXIF_RAW,roundtrip,"ExifRaw",&raw) && FreeImage_GetTagLength(raw)==sizeof(exif), "raw EXIF roundtrip");
        Require(FreeImage_GetMetadata(FIMD_EXIF_MAIN,roundtrip,"Orientation",&raw), "parsed EXIF");
        FreeImage_Unload(roundtrip); FreeImage_CloseMemory(encoded);
        encoded = FreeImage_OpenMemory();
        Require(FreeImage_SaveToMemory(format,dib,encoded,HEIF_LOSSLESS), "lossless encode");
        FreeImage_SeekMemory(encoded,0,SEEK_SET); roundtrip=FreeImage_LoadFromMemory(format,encoded);
        Require(roundtrip!=NULL,"lossless decode");
        for(unsigned y=0;y<24;++y) Require(!std::memcmp(FreeImage_GetScanLine(dib,y),FreeImage_GetScanLine(roundtrip,y),FreeImage_GetLine(dib)),"exact lossless pixels");
        FreeImage_Unload(roundtrip); FreeImage_CloseMemory(encoded);
    }
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
int main(int argc, char **) {
    encode = argc > 1;
    FreeImage_Initialise();
    Require(FreeImage_GetFIFFromFormat("HEIF") == FIF_HEIF, "HEIF index");
    Require(FreeImage_GetFIFFromFormat("AVIF") == FIF_AVIF, "AVIF index");

    Test("rgb.heic", FIF_HEIF, false);
    Test("rgba.heic", FIF_HEIF, true);
    Test("rgb.avif", FIF_AVIF, false);
    Test("rgba.avif", FIF_AVIF, true);
    Test("metadata.heic", FIF_HEIF, false);
    FreeImage_DeInitialise();
    std::puts("HEIF/AVIF RGB, alpha, orientation, header-only and truncated-input tests passed");
}
