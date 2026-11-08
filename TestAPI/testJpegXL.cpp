#include "FreeImage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#if defined(_WIN32)
#include <malloc.h>
#define JXL_TEST_ALLOCA _alloca
#else
#include <alloca.h>
#define JXL_TEST_ALLOCA alloca
#endif
static void Check(bool v) { if(!v) { std::fprintf(stderr,"JXL regression failed\n"); std::exit(1); } }
#if FREEIMAGE_TEST_JXL_EXPORT
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
#endif
// Lossy files from cjxl run VarDCT and the upsampling stages' SIMD row code,
// which a lossless roundtrip never reaches. Issue #211 crashed in that
// stage's AVX2 constructor on MinGW GCC (every decode builds an 8x upsampler).
static void TestFixture(const char *dir,const char *name,FIBITMAP *reference,double max_error) {
    char path[1024]; std::snprintf(path,sizeof(path),"%s/%s",dir,name);
    Check(FreeImage_GetFileType(path,0)==FIF_JPEGXL);
    FIBITMAP *decoded=FreeImage_Load(FIF_JPEGXL,path,0);
    if(!decoded) { std::fprintf(stderr,"%s: load failed\n",name); std::exit(1); }
    Check(FreeImage_GetWidth(decoded)==FreeImage_GetWidth(reference) && FreeImage_GetHeight(decoded)==FreeImage_GetHeight(reference) && FreeImage_GetBPP(decoded)==32);
    // Lossy encoders may change the color under fully transparent pixels, so
    // weight color error by the reference alpha.
    double error=0;
    for(unsigned y=0;y<FreeImage_GetHeight(reference);++y) {
        const BYTE *a=FreeImage_GetScanLine(decoded,y), *b=FreeImage_GetScanLine(reference,y);
        for(unsigned x=0;x<FreeImage_GetWidth(reference);++x,a+=4,b+=4) {
            const double weight=b[FI_RGBA_ALPHA]/255.0;
            error+=std::fabs(double(a[FI_RGBA_ALPHA])-double(b[FI_RGBA_ALPHA]));
            for(int c=0;c<3;++c) error+=weight*std::fabs(double(a[c])-double(b[c]));
        }
    }
    error/=double(FreeImage_GetWidth(reference))*FreeImage_GetHeight(reference)*4;
    if(error>max_error) { std::fprintf(stderr,"%s: error %.2f exceeds %.2f\n",name,error,max_error); std::exit(1); }
    FreeImage_Unload(decoded);
}
// Stack alignment inside the decoder depends only on the caller's depth, so
// repeating a call from the same frame repeats the same alignment. Offset the
// stack by 16-byte steps to land every 32- and 64-byte alignment class.
static void TestFixturesShifted(const char *dir,FIBITMAP *reference,unsigned shift) {
    volatile char *pad=static_cast<volatile char *>(JXL_TEST_ALLOCA(16*shift+16)); pad[0]=0;
    TestFixture(dir,"lossy.jxl",reference,3); TestFixture(dir,"progressive.jxl",reference,3);
    TestFixture(dir,"upsampled2x.jxl",reference,12); TestFixture(dir,"upsampled4x.jxl",reference,18);
    TestFixture(dir,"alpha-upsampled.jxl",reference,9);
    pad[0]=1;
}
#if !FREEIMAGE_TEST_JXL_EXPORT
// Built with FREEIMAGE_JXL_EXPORT=OFF: the plugin must be read-only, and
// header-only and truncated loads are checked on a fixture instead.
static void TestReadOnly(const char *dir) {
    Check(!FreeImage_FIFSupportsWriting(FIF_JPEGXL) && !FreeImage_FIFSupportsExportBPP(FIF_JPEGXL,24));
    FIBITMAP *src=FreeImage_Allocate(17,9,24); Check(src!=NULL);
    FIMEMORY *memory=FreeImage_OpenMemory(); Check(!FreeImage_SaveToMemory(FIF_JPEGXL,src,memory,JXL_LOSSLESS));
    FreeImage_CloseMemory(memory); FreeImage_Unload(src);
    if(!dir) return;
    char path[1024]; std::snprintf(path,sizeof(path),"%s/lossy.jxl",dir);
    FIBITMAP *header=FreeImage_Load(FIF_JPEGXL,path,FIF_LOAD_NOPIXELS); Check(header && !FreeImage_HasPixels(header)); FreeImage_Unload(header);
    FILE *file=std::fopen(path,"rb"); Check(file!=NULL);
    BYTE bytes[65536]; const size_t size=std::fread(bytes,1,sizeof(bytes),file); std::fclose(file); Check(size>16 && size<sizeof(bytes));
    FIMEMORY *truncated=FreeImage_OpenMemory(bytes,static_cast<DWORD>(size/2)); Check(FreeImage_LoadFromMemory(FIF_JPEGXL,truncated)==NULL); FreeImage_CloseMemory(truncated);
}
#endif
int main(int argc,char **argv) {
    FreeImage_Initialise(); Check(FreeImage_GetFIFFromFormat("JXL")==FIF_JPEGXL);
#if FREEIMAGE_TEST_JXL_EXPORT
    Test(FIT_BITMAP,24); Test(FIT_BITMAP,32); Test(FIT_RGB16,48); Test(FIT_RGBF,96);
#else
    TestReadOnly(argc>1 ? argv[1] : NULL);
#endif
    if(argc>1) {
        char path[1024]; std::snprintf(path,sizeof(path),"%s/reference.png",argv[1]);
        FIBITMAP *reference=FreeImage_Load(FIF_PNG,path,0); Check(reference && FreeImage_GetBPP(reference)==32);
        for(unsigned shift=0;shift<4;++shift) TestFixturesShifted(argv[1],reference,shift);
        FreeImage_Unload(reference);
    }
    FreeImage_DeInitialise(); std::puts("JXL exact lossless RGB/RGBA/16-bit/float roundtrips, headers, truncation and lossy/upsampled decodes passed");
}
