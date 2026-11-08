#include "FreeImage.h"
#include <cstdio>
#include <cstring>
#include <vector>
extern "C" const char *libraw_version(void);
static bool roundtrip(FREE_IMAGE_FORMAT format, int flags, bool exact) {
    FIBITMAP* image=FreeImage_Allocate(32,32,24);
    FIMEMORY* stream=FreeImage_OpenMemory();
    if (!image || !stream) return false;
    RGBQUAD expected{}; expected.rgbRed=101; expected.rgbGreen=55; expected.rgbBlue=201;
    for (unsigned y=0;y<32;++y)
        for (unsigned x=0;x<32;++x) FreeImage_SetPixelColor(image,x,y,&expected);
    bool ok=FreeImage_SaveToMemory(format,image,stream,flags)!=0;
    FreeImage_SeekMemory(stream,0,SEEK_SET);
    FIBITMAP* loaded=ok ? FreeImage_LoadFromMemory(format,stream) : nullptr;
    RGBQUAD actual{};
    ok=loaded && FreeImage_GetWidth(loaded)==32 && FreeImage_GetHeight(loaded)==32 && FreeImage_GetPixelColor(loaded,15,15,&actual);
    if (ok) {
        int tolerance=exact ? 0 : 3;
        for (auto pair : {std::pair<int,int>{expected.rgbRed,actual.rgbRed},{expected.rgbGreen,actual.rgbGreen},{expected.rgbBlue,actual.rgbBlue}})
            ok=ok && pair.first-pair.second<=tolerance && pair.second-pair.first<=tolerance;
    }
    if (loaded) FreeImage_Unload(loaded);
    FreeImage_CloseMemory(stream); FreeImage_Unload(image);
    std::printf("format %d memory round trip: %s\n",format,ok ? "PASS" : "FAIL");
    return ok;
}
int main() {
    FreeImage_Initialise();
    bool ok=roundtrip(FIF_PNG,0,true);
    ok=roundtrip(FIF_JPEG,JPEG_QUALITYSUPERB,false) && ok;
    ok=roundtrip(FIF_WEBP,WEBP_LOSSLESS,true) && ok;
    ok=roundtrip(FIF_TIFF,TIFF_JPEG,false) && ok;
    std::vector<BYTE> source(10000),compressed(11000),output(source.size());
    for (size_t i=0;i<source.size();++i) source[i]=(BYTE)(i%251);
    DWORD count=FreeImage_ZLibCompress(compressed.data(),(DWORD)compressed.size(),source.data(),(DWORD)source.size());
    DWORD decoded=FreeImage_ZLibUncompress(output.data(),(DWORD)output.size(),compressed.data(),count);
    bool zlib=count && decoded==source.size() && output==source;
    std::printf("zlib API round trip: %s\n",zlib ? "PASS" : "FAIL");
    ok=ok && zlib;
    const char* version=libraw_version();
    bool raw=version && !std::strncmp(version,"0.22.2",6);
    std::printf("Linked LibRaw: %s\n",version ? version : "missing");
    ok=ok && raw;
    FreeImage_DeInitialise();
    return ok ? 0 : 1;
}
