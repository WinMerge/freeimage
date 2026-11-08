#include "FreeImage.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static void u16(std::vector<BYTE>& b, size_t p, unsigned v) {
    b[p] = (BYTE)v; b[p+1] = (BYTE)(v >> 8);
}
static void u32(std::vector<BYTE>& b, size_t p, unsigned v) {
    for (unsigned i=0; i<4; ++i) b[p+i] = (BYTE)(v >> (8*i));
}
static FIBITMAP* load(FREE_IMAGE_FORMAT format, std::vector<BYTE>& b) {
    FIMEMORY* memory = FreeImage_OpenMemory(b.data(), (DWORD)b.size());
    FIBITMAP* dib = FreeImage_LoadFromMemory(format, memory);
    FreeImage_CloseMemory(memory);
    return dib;
}
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    FreeImage_Initialise();
    std::string test(argv[1]);
    FIBITMAP* dib = nullptr;
    bool ok = false;
    if (test == "cut-rle-bounds") {
        std::vector<BYTE> b(8, 0);
        u16(b, 0, 0x230b); u16(b, 2, 1);
        b.insert(b.end(), {0,0,0,0x81,0x7f});
        dib = load(FIF_CUT, b);
        // The port preserves the existing partial-image return behavior.
        ok = dib != nullptr;
    } else if (test == "pcx-plane-width") {
        std::vector<BYTE> b(128, 0);
        b[0]=10; b[1]=5; b[3]=8;
        u16(b,8,3); u16(b,10,1); b[65]=3; u16(b,66,2);
        b.insert(b.end(), {1,2,3,4,5,6,1,2,3,4,5,6});
        dib = load(FIF_PCX, b);
        ok = dib == nullptr;
        // A correctly sized control must still decode.
        u16(b,8,1);
        FIBITMAP* control = load(FIF_PCX,b);
        ok = ok && control && FreeImage_GetWidth(control)==2;
        if (control) FreeImage_Unload(control);
    } else if (test == "dds-partial-blocks") {
        std::vector<BYTE> b(128,0);
        std::memcpy(b.data(),"DDS ",4);
        u32(b,4,124); u32(b,8,0x81007); u32(b,12,5); u32(b,16,5);
        u32(b,20,32); u32(b,76,32); u32(b,80,4);
        std::memcpy(b.data()+84,"DXT1",4); u32(b,108,0x1000);
        for (int i=0;i<4;++i) b.insert(b.end(),{0,0xf8,0,0,0,0,0,0});
        dib = load(FIF_DDS,b);
        ok = dib && FreeImage_GetWidth(dib)==5 && FreeImage_GetHeight(dib)==5;
        if (ok) {
            RGBQUAD pixel{};
            ok = FreeImage_GetPixelColor(dib,4,4,&pixel) && pixel.rgbRed==255;
        }
    } else if (test == "rgb565-conversion") {
        FIBITMAP* input = FreeImage_Allocate(1,1,16,0,FI16_565_GREEN_MASK,FI16_565_BLUE_MASK);
        if (!input) return 2;
        WORD pixel=0x07e0;
        std::memcpy(FreeImage_GetBits(input),&pixel,sizeof(pixel));
        dib = FreeImage_ConvertTo32Bits(input);
        RGBQUAD color{};
        ok = dib && FreeImage_GetPixelColor(dib,0,0,&color) && color.rgbGreen==255 && color.rgbRed==0;
        FreeImage_Unload(input);
    } else if (test == "bmp-extended-headers") {
        ok = true;
        for (unsigned size : {40u,52u,56u,108u,124u}) {
            std::vector<BYTE> b(14+size+4,0);
            b[0]='B'; b[1]='M'; u32(b,2,(unsigned)b.size()); u32(b,10,14+size);
            u32(b,14,size); u32(b,18,1); u32(b,22,1); u16(b,26,1); u16(b,28,24);
            b[14+size+2]=255;
            FIBITMAP* image = load(FIF_BMP,b);
            ok = ok && image && FreeImage_GetWidth(image)==1;
            if (image) FreeImage_Unload(image);
        }
    } else if (test == "bmp-rle8-terminator") {
        std::vector<BYTE> b(14+40+8,0);
        b[0]='B'; b[1]='M'; u32(b,10,(unsigned)b.size());
        u32(b,14,40); u32(b,18,1); u32(b,22,1); u16(b,26,1); u16(b,28,8);
        u32(b,30,1); u32(b,34,6); u32(b,46,2);
        b[58]=b[59]=b[60]=255;
        b.insert(b.end(),{1,1,0,0,0,1}); u32(b,2,(unsigned)b.size());
        dib = load(FIF_BMP,b);
        ok = dib && FreeImage_GetWidth(dib)==1;
        // A run after the last scanline must still be rejected.
        b[b.size()-2]=1; b[b.size()-1]=1;
        FIBITMAP* malformed=load(FIF_BMP,b);
        ok = ok && !malformed;
        if (malformed) FreeImage_Unload(malformed);
    } else return 2;
    if (dib) FreeImage_Unload(dib);
    FreeImage_DeInitialise();
    std::printf("%s: %s\n",argv[1],ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
