#include "FreeImage.h"
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdlib>
static void Check(bool value) { if(!value) std::exit(1); }
int main() {
    FIBITMAP *src=FreeImage_AllocateT(FIT_FLOAT,5,2); Check(src!=NULL);
    const float samples[]={-1,0,.5f,1,std::numeric_limits<float>::quiet_NaN()};
    for(unsigned y=0;y<2;++y) for(unsigned x=0;x<5;++x) reinterpret_cast<float *>(FreeImage_GetScanLine(src,y))[x]=samples[x];
    double low=99,high=99; void *minp=NULL,*maxp=NULL;
    Check(FreeImage_FindMinMax(src,&low,&high,&minp,&maxp) && low==-1 && high==1);
    Check(*static_cast<float *>(minp)==-1 && *static_cast<float *>(maxp)==1);
    float minv=0,maxv=0; Check(FreeImage_FindMinMaxValue(src,&minv,&maxv) && minv==-1 && maxv==1);
    DWORD histogram[4]={99,99,99,99};
    Check(FreeImage_GetHistogramEx(src,histogram,4,0,1,FICC_RGB));
    Check(histogram[0]==2 && histogram[1]==0 && histogram[2]==2 && histogram[3]==2);
    Check(!FreeImage_GetHistogramEx(src,histogram,0,0,1,FICC_RGB));
    FIBITMAP *clamp=FreeImage_TmoClamp(src),*linear=FreeImage_TmoLinear(src); Check(clamp && linear);
    Check(FreeImage_GetScanLine(clamp,0)[0]==0 && FreeImage_GetScanLine(clamp,0)[2]==128 && FreeImage_GetScanLine(clamp,0)[3]==255);
    Check(FreeImage_GetScanLine(linear,0)[1]==128 && FreeImage_GetScanLine(linear,0)[3]==255);
    FreeImage_Unload(clamp); FreeImage_Unload(linear); FreeImage_Unload(src);
    src=FreeImage_AllocateT(FIT_RGBAF,2,1); FIRGBAF *pixels=reinterpret_cast<FIRGBAF *>(FreeImage_GetBits(src));
    pixels[0]={2,1,.5f,.25f}; pixels[1]={4,2,1,.75f};
    linear=FreeImage_ToneMapping(src,FITMO_LINEAR); Check(linear!=NULL);
    Check(FreeImage_GetBits(linear)[FI_RGBA_ALPHA]==64 && FreeImage_GetBits(linear)[4+FI_RGBA_ALPHA]==191);
    FreeImage_Unload(linear); FreeImage_Unload(src);
    src=FreeImage_AllocateT(FIT_DOUBLE,2,1);
    double *values=reinterpret_cast<double *>(FreeImage_GetBits(src)); values[0]=values[1]=3;
    linear=FreeImage_TmoLinear(src); Check(linear && FreeImage_GetBits(linear)[0]==0);
    FreeImage_Unload(linear); values[0]=values[1]=std::numeric_limits<double>::quiet_NaN();
    low=7; high=8; Check(!FreeImage_FindMinMax(src,&low,&high) && low==7 && high==8);
    FreeImage_Unload(src); std::puts("Min/max, HDR histograms, tone mapping, alpha and NaN tests passed");
}
