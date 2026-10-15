// Visual proof: all loading, pixel access, compositing and PNG export use FreeImage.
#include <FreeImage.h>
#include <cmath>
#include <cstdio>
#include <cstring>

int main(int argc, char **argv) {
    if(argc != 4) {
        std::fprintf(stderr, "Usage: HeifAvifPreview prepare|decode input output.png\n");
        return 1;
    }
    FreeImage_Initialise();
    const FREE_IMAGE_FORMAT format = FreeImage_GetFileType(argv[2]);
    FIBITMAP *loaded = FreeImage_Load(format, argv[2]);
    if(!loaded) return 2;
    std::printf("FreeImage_Load(%s): %u x %u, %u bpp\n", FreeImage_GetFormatFromFIF(format),
        FreeImage_GetWidth(loaded), FreeImage_GetHeight(loaded), FreeImage_GetBPP(loaded));
    FIBITMAP *rgba = FreeImage_ConvertTo32Bits(loaded);
    FreeImage_Unload(loaded);
    if(!rgba) return 3;
    const unsigned w = FreeImage_GetWidth(rgba), h = FreeImage_GetHeight(rgba);
    if(!std::strcmp(argv[1], "prepare")) {
        // Reuse the repository's sample photograph, adding a soft circular alpha mask.
        for(unsigned y = 0; y < h; ++y) for(unsigned x = 0; x < w; ++x) {
            const double dx = (double(x) - w / 2.0) / (w / 2.0);
            const double dy = (double(y) - h / 2.0) / (h / 2.0);
            const double coverage = (1.0 - std::sqrt(dx * dx + dy * dy)) * 6.0;
            FreeImage_GetScanLine(rgba, y)[x * 4 + FI_RGBA_ALPHA] =
                static_cast<BYTE>(coverage <= 0 ? 0 : coverage >= 1 ? 255 : coverage * 255);
        }
        const bool ok = FreeImage_Save(FIF_PNG, rgba, argv[3]) != 0;
        FreeImage_Unload(rgba); FreeImage_DeInitialise(); return ok ? 0 : 4;
    }
    FIBITMAP *preview = FreeImage_Allocate(w, h, 24);
    if(!preview) return 5;
    for(unsigned y = 0; y < h; ++y) for(unsigned x = 0; x < w; ++x) {
        const BYTE *src = FreeImage_GetScanLine(rgba, y) + x * 4;
        BYTE *dst = FreeImage_GetScanLine(preview, y) + x * 3;
        const unsigned background = ((x / 12 + y / 12) % 2) ? 210 : 245;
        const unsigned alpha = src[FI_RGBA_ALPHA];
        for(unsigned c = 0; c < 3; ++c)
            dst[c] = static_cast<BYTE>((src[c] * alpha + background * (255 - alpha) + 127) / 255);
    }
    FIBITMAP *large = FreeImage_Rescale(preview, w * 2, h * 2, FILTER_BOX);
    const bool ok = large && FreeImage_Save(FIF_PNG, large, argv[3]);
    FreeImage_Unload(large); FreeImage_Unload(preview); FreeImage_Unload(rgba);
    FreeImage_DeInitialise(); return ok ? 0 : 6;
}
