#include <FreeImage.h>
#include <cstdio>
#include <cstring>
// Optional arguments: expected HEIF/AVIF and JPEG XL writer support (0 or 1),
// checked against the installed library.
int main(int argc, char **argv) {
    FreeImage_Initialise();
    const bool supported=FreeImage_FIFSupportsReading(FIF_HEIF) && FreeImage_FIFSupportsReading(FIF_AVIF)
        && FreeImage_FIFSupportsReading(FIF_JPEGXL);
    const int heif_writes=FreeImage_FIFSupportsWriting(FIF_HEIF) && FreeImage_FIFSupportsWriting(FIF_AVIF) ? 1 : 0;
    const int jxl_writes=FreeImage_FIFSupportsWriting(FIF_JPEGXL) ? 1 : 0;
    std::printf("FreeImage %s: HEIF/AVIF/JXL enabled=%d, HEIF/AVIF writes=%d, JXL writes=%d\n",
        FreeImage_GetVersion(),supported,heif_writes,jxl_writes);
    FreeImage_DeInitialise();
    bool expected=true;
    if(argc>2) {
        if(heif_writes!=(std::strcmp(argv[1],"1")==0 ? 1 : 0)) { std::fprintf(stderr,"HEIF/AVIF writer support is not %s\n",argv[1]); expected=false; }
        if(jxl_writes!=(std::strcmp(argv[2],"1")==0 ? 1 : 0)) { std::fprintf(stderr,"JXL writer support is not %s\n",argv[2]); expected=false; }
    }
    return supported && expected ? 0 : 1;
}
