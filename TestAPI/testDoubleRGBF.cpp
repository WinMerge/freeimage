#include "FreeImage.h"
#include <cmath>
#include <limits>
#include <cstdio>
int main() {
    FIBITMAP *src = FreeImage_AllocateT(FIT_DOUBLE, 5, 2);
    if(!src) return 1;
    double values[] = {-2, .5, 4, std::numeric_limits<double>::max(), std::numeric_limits<double>::quiet_NaN()};
    for(unsigned y=0;y<2;++y) for(unsigned x=0;x<5;++x) reinterpret_cast<double *>(FreeImage_GetScanLine(src,y))[x]=values[x];
    FIBITMAP *dst = FreeImage_ConvertToRGBF(src);
    if(!dst || FreeImage_GetImageType(dst)!=FIT_RGBF) return 2;
    for(unsigned y=0;y<2;++y) {
        FIRGBF *row=reinterpret_cast<FIRGBF *>(FreeImage_GetScanLine(dst,y));
        for(unsigned x=0;x<5;++x) {
            if(x==4) { if(!std::isnan(row[x].red)) return 3; }
            else if(x==3) { if(!std::isinf(row[x].red)) return 4; }
            else if(row[x].red!=values[x] || row[x].green!=values[x] || row[x].blue!=values[x]) return 5;
        }
    }
    if(reinterpret_cast<double *>(FreeImage_GetScanLine(src,0))[0]!=-2) return 6;
    FreeImage_Unload(dst); FreeImage_Unload(src);
    std::puts("Double grayscale preserves HDR values in RGBF conversion");
}
