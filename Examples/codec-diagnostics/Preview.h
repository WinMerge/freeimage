#ifndef FREEIMAGE_EXAMPLE_PREVIEW_H
#define FREEIMAGE_EXAMPLE_PREVIEW_H
#include <FreeImage.h>

namespace codec_example {
// Explicit RGB display preview. Native pixels are retained by the caller.
// HDR tone mapping changes appearance; the 24-bit result discards alpha.
inline FIBITMAP *MakeRGBPreview(FIBITMAP *source) {
    if(!FreeImage_HasPixels(source)) return NULL;
    switch(FreeImage_GetImageType(source)) {
        case FIT_BITMAP:
        case FIT_RGB16:
        case FIT_RGBA16:
            return FreeImage_ConvertTo24Bits(source);
        case FIT_RGBF:
        case FIT_RGBAF:
            return FreeImage_ToneMapping(source, FITMO_DRAGO03);
        default:
            return NULL;
    }
}
}
#endif
