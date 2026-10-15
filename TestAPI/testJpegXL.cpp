#include "FreeImage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
static void Check(bool v) { if(!v) { std::fprintf(stderr,"JXL regression failed\n"); std::exit(1); } }
static void Test(FREE_IMAGE_TYPE type,int bpp) {
    FIBITMAP *src=FreeImage_AllocateT(type,17,9,bpp); Check(src!=NULL);
    for(unsigned y=0;y<9;++y) {
        BYTE *p=FreeImage_GetScanLine(src,y);
        if(type==FIT_BITMAP) for(unsigned x=0;x<17;++x) { p[x*(bpp/8)+FI_RGBA_RED]=static_cast<BYTE>(x*11); p[x*(bpp/8)+FI_RGBA_GREEN]=static_cast<BYTE>(y*23); p[x*(bpp/8)+FI_RGBA_BLUE]=70; if(bpp==32) p[x*4+FI_RGBA_ALPHA]=static_cast<BYTE>(50+x*10); }
        else if(type==FIT_RGB16) for(unsigned x=0;x<17*3;++x) reinterpret_cast<WORD *>(p)[x]=static_cast<WORD>(x*1000+y);
        else for(unsigned x=0;x<17*3;++x) reinterpret_cast<float *>(p)[x]=float(x)/17;
    }
    FIMEMORY *memory=FreeImage_OpenMemory(); Check(FreeImage_SaveToMemory(FIF_JPEGXL,src,memory,JXL_LOSSLESS));
    FreeImage_SeekMemory(memory,0,SEEK_SET); Check(FreeImage_GetFileTypeFromMemory(memory)==FIF_JPEGXL);
    FIBITMAP *decoded=FreeImage_LoadFromMemory(FIF_JPEGXL,memory); Check(decoded && FreeImage_GetImageType(decoded)==type && FreeImage_GetWidth(decoded)==17);
    for(unsigned y=0;y<9;++y) Check(!std::memcmp(FreeImage_GetScanLine(src,y),FreeImage_GetScanLine(decoded,y),FreeImage_GetLine(src)));
    FreeImage_Unload(decoded); FreeImage_SeekMemory(memory,0,SEEK_SET);
    decoded=FreeImage_LoadFromMemory(FIF_JPEGXL,memory,FIF_LOAD_NOPIXELS); Check(decoded && !FreeImage_HasPixels(decoded)); FreeImage_Unload(decoded);
    BYTE *bytes=NULL; DWORD size=0; Check(FreeImage_AcquireMemory(memory,&bytes,&size));
    FIMEMORY *truncated=FreeImage_OpenMemory(bytes,size/2); Check(FreeImage_LoadFromMemory(FIF_JPEGXL,truncated)==NULL); FreeImage_CloseMemory(truncated);
    FreeImage_CloseMemory(memory); FreeImage_Unload(src);
}
int main() {
    FreeImage_Initialise(); Check(FreeImage_GetFIFFromFormat("JXL")==FIF_JPEGXL);
    Test(FIT_BITMAP,24); Test(FIT_BITMAP,32); Test(FIT_RGB16,48); Test(FIT_RGBF,96);
    FreeImage_DeInitialise(); std::puts("JXL exact lossless RGB/RGBA/16-bit/float roundtrips, headers and truncation passed");
}
