// Image-analysis and simple tone-map extensions inspired by FreeImageRe.
// This implementation uses the existing FreeImage 3 pixel types and C++11.
#include "FreeImage.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <vector>
namespace {
struct Layout { unsigned channels; double scale; };
bool GetLayout(FIBITMAP *dib, Layout &layout) {
    if(!FreeImage_HasPixels(dib)) return false;
    switch(FreeImage_GetImageType(dib)) {
    case FIT_BITMAP:
        if(FreeImage_GetColorType(dib)==FIC_CMYK) return false;
        if(FreeImage_GetBPP(dib)==8 && FreeImage_GetColorType(dib)==FIC_MINISBLACK) layout={1,255};
        else if(FreeImage_GetBPP(dib)==24) layout={3,255};
        else if(FreeImage_GetBPP(dib)==32) layout={4,255};
        else return false;
        return true;
    case FIT_UINT16: layout={1,65535}; return true;
    case FIT_RGB16: layout={3,65535}; return true;
    case FIT_RGBA16: layout={4,65535}; return true;
    case FIT_FLOAT: case FIT_DOUBLE: layout={1,1}; return true;
    case FIT_RGBF: layout={3,1}; return true;
    case FIT_RGBAF: layout={4,1}; return true;
    default: return false;
    }
}
BYTE *Pixel(FIBITMAP *dib,unsigned x,unsigned y) { return FreeImage_GetScanLine(dib,y)+x*(FreeImage_GetBPP(dib)/8); }
void Read(FIBITMAP *dib,unsigned x,unsigned y,const Layout &l,double *v) {
    const BYTE *p=Pixel(dib,x,y);
    switch(FreeImage_GetImageType(dib)) {
    case FIT_BITMAP:
        if(l.channels==1) v[0]=p[0];
        else { v[0]=p[FI_RGBA_RED]; v[1]=p[FI_RGBA_GREEN]; v[2]=p[FI_RGBA_BLUE]; if(l.channels==4) v[3]=p[FI_RGBA_ALPHA]; }
        break;
    case FIT_UINT16: case FIT_RGB16: case FIT_RGBA16:
        for(unsigned c=0;c<l.channels;++c) v[c]=reinterpret_cast<const WORD *>(p)[c]; break;
    case FIT_FLOAT: case FIT_RGBF: case FIT_RGBAF:
        for(unsigned c=0;c<l.channels;++c) v[c]=reinterpret_cast<const float *>(p)[c]; break;
    case FIT_DOUBLE: v[0]=*reinterpret_cast<const double *>(p); break;
    default: break;
    }
}
double Brightness(const double *v,const Layout &l) { return l.channels==1 ? v[0] : .2126*v[0]+.7152*v[1]+.0722*v[2]; }
BYTE Clamp(double value) { return std::isnan(value) || value<=0 ? 0 : value>=1 ? 255 : static_cast<BYTE>(value*255+.5); }
FIBITMAP *Tone(FIBITMAP *src,bool linear,double maximum) {
    Layout l; if(!GetLayout(src,l) || !std::isfinite(maximum)) return NULL;
    double minimum=0, high=maximum>0 ? maximum : l.scale;
    if(linear) {
        double low,top;
        if(!FreeImage_FindMinMax(src,&low,&top,NULL,NULL)) return NULL;
        minimum=low; high=maximum>0 ? maximum : top;
        if(high<minimum) return NULL;
    }
    FIBITMAP *dst=FreeImage_Allocate(FreeImage_GetWidth(src),FreeImage_GetHeight(src),l.channels==1 ? 8 : l.channels*8);
    if(!dst) return NULL;
    if(l.channels==1) for(unsigned i=0;i<256;++i) { RGBQUAD *p=FreeImage_GetPalette(dst)+i; p->rgbRed=p->rgbGreen=p->rgbBlue=static_cast<BYTE>(i); }
    for(unsigned y=0;y<FreeImage_GetHeight(src);++y) for(unsigned x=0;x<FreeImage_GetWidth(src);++x) {
        double v[4]={}; Read(src,x,y,l,v); BYTE *out=Pixel(dst,x,y); BYTE colour[4]={};
        for(unsigned c=0;c<l.channels;++c) colour[c]=c==3 ? Clamp(v[c]/l.scale) : Clamp(high==minimum ? 0 : (v[c]-minimum)/(high-minimum));
        if(l.channels==1) out[0]=colour[0];
        else { out[FI_RGBA_RED]=colour[0]; out[FI_RGBA_GREEN]=colour[1]; out[FI_RGBA_BLUE]=colour[2]; if(l.channels==4) out[FI_RGBA_ALPHA]=colour[3]; }
    }
    FreeImage_CloneMetadata(dst,src); return dst;
}
void Store(FREE_IMAGE_TYPE type,unsigned channels,const double *v,void *out) {
    if(!out) return;
    for(unsigned c=0;c<channels;++c) {
        if(type==FIT_BITMAP) {
            if(channels==1) static_cast<BYTE *>(out)[0]=static_cast<BYTE>(v[0]);
            else { const unsigned offsets[]={FI_RGBA_RED,FI_RGBA_GREEN,FI_RGBA_BLUE,FI_RGBA_ALPHA}; static_cast<BYTE *>(out)[offsets[c]]=static_cast<BYTE>(v[c]); }
        } else if(type==FIT_UINT16 || type==FIT_RGB16 || type==FIT_RGBA16) static_cast<WORD *>(out)[c]=static_cast<WORD>(v[c]);
        else if(type==FIT_DOUBLE) static_cast<double *>(out)[c]=v[c];
        else static_cast<float *>(out)[c]=static_cast<float>(v[c]);
    }
}
}
BOOL DLL_CALLCONV FreeImage_FindMinMax(FIBITMAP *dib,double *low,double *high,void **low_pixel,void **high_pixel) {
    Layout l; if(!GetLayout(dib,l)) return FALSE;
    bool found=false; double minimum=0,maximum=0; BYTE *minp=NULL,*maxp=NULL;
    for(unsigned y=0;y<FreeImage_GetHeight(dib);++y) for(unsigned x=0;x<FreeImage_GetWidth(dib);++x) {
        double v[4]={}; Read(dib,x,y,l,v); const double value=Brightness(v,l); if(!std::isfinite(value)) continue;
        if(!found || value<minimum) { minimum=value; minp=Pixel(dib,x,y); }
        if(!found || value>maximum) { maximum=value; maxp=Pixel(dib,x,y); }
        found=true;
    }
    if(!found) return FALSE;
    if(low) *low=minimum; if(high) *high=maximum; if(low_pixel) *low_pixel=minp; if(high_pixel) *high_pixel=maxp;
    return TRUE;
}
BOOL DLL_CALLCONV FreeImage_FindMinMaxValue(FIBITMAP *dib,void *low,void *high) {
    Layout l; if(!GetLayout(dib,l) || (!low && !high)) return FALSE;
    double minimum[4]={},maximum[4]={}; bool found[4]={};
    for(unsigned y=0;y<FreeImage_GetHeight(dib);++y) for(unsigned x=0;x<FreeImage_GetWidth(dib);++x) {
        double v[4]={}; Read(dib,x,y,l,v);
        for(unsigned c=0;c<l.channels;++c) if(std::isfinite(v[c])) { if(!found[c] || v[c]<minimum[c]) minimum[c]=v[c]; if(!found[c] || v[c]>maximum[c]) maximum[c]=v[c]; found[c]=true; }
    }
    for(unsigned c=0;c<l.channels;++c) if(!found[c]) return FALSE;
    Store(FreeImage_GetImageType(dib),l.channels,minimum,low); Store(FreeImage_GetImageType(dib),l.channels,maximum,high); return TRUE;
}
FIBITMAP * DLL_CALLCONV FreeImage_TmoClamp(FIBITMAP *src,double maximum) { return Tone(src,false,maximum); }
FIBITMAP * DLL_CALLCONV FreeImage_TmoLinear(FIBITMAP *src,double maximum) { return Tone(src,true,maximum); }
BOOL DLL_CALLCONV FreeImage_GetHistogramEx(FIBITMAP *dib,DWORD *histogram,unsigned bins,double low,double high,FREE_IMAGE_COLOR_CHANNEL channel) {
    Layout l; if(!GetLayout(dib,l) || !histogram || !bins || !std::isfinite(low) || !std::isfinite(high) || low>=high || !std::isfinite(high-low)) return FALSE;
    unsigned component=0;
    if(channel!=FICC_RGB) {
        if(channel==FICC_RED) component=0; else if(channel==FICC_GREEN) component=1; else if(channel==FICC_BLUE) component=2; else if(channel==FICC_ALPHA) component=3; else return FALSE;
        if(component>=l.channels) return FALSE;
    }
    try {
        std::vector<DWORD> values(bins,0);
        for(unsigned y=0;y<FreeImage_GetHeight(dib);++y) for(unsigned x=0;x<FreeImage_GetWidth(dib);++x) {
            double v[4]={}; Read(dib,x,y,l,v); const double value=channel==FICC_RGB ? Brightness(v,l) : v[component];
            if(!std::isfinite(value) || value<low || value>high) continue;
            const unsigned bin=value==high ? bins-1 : std::min(bins-1,static_cast<unsigned>(((value-low)/(high-low))*bins));
            if(values[bin]==std::numeric_limits<DWORD>::max()) return FALSE; ++values[bin];
        }
        std::memcpy(histogram,values.data(),size_t(bins)*sizeof(DWORD)); return TRUE;
    } catch(...) { return FALSE; }
}
