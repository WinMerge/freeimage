#include <FreeImage.h>
#include <cstdio>
int main() {
    FreeImage_Initialise();
    const bool supported=FreeImage_FIFSupportsReading(FIF_HEIF) && FreeImage_FIFSupportsReading(FIF_AVIF)
        && FreeImage_FIFSupportsReading(FIF_JPEGXL);
    std::printf("FreeImage %s: HEIF/AVIF/JXL enabled=%d\n",FreeImage_GetVersion(),supported);
    FreeImage_DeInitialise();
    return supported ? 0 : 1;
}
