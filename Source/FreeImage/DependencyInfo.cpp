// Additive dependency diagnostics, inspired by FreeImageRe's dependency API.
// Strings are library-owned; null means that no reliable version API exists.
#include "FreeImage.h"
#include <cstdio>
#include <string>
// libpng and the public API use this name for different constants.
#undef PNG_Z_DEFAULT_COMPRESSION
#ifdef FREEIMAGE_SYSTEM_ZLIB
#include <zlib.h>
#else
#include "../ZLib/zlib.h"
#endif
#ifdef FREEIMAGE_SYSTEM_LIBPNG
#include <png.h>
#else
#include "../LibPNG/png.h"
#endif
#ifdef FREEIMAGE_SYSTEM_LIBJPEG
#include <jpeglib.h>
#else
#include "../LibJPEG/jpeglib.h"
#endif
#if defined(INCLUDE_LIB_TIFF4) || defined(FREEIMAGE_SYSTEM_LIBTIFF)
#include <tiffio.h>
#endif
#include <openjpeg.h>
#ifdef INCLUDE_LIB_WEBP
#include <webp/decode.h>
#endif
#ifdef INCLUDE_LIB_RAW
#include <libraw/libraw_version.h>
extern "C" const char *libraw_version(void);
#endif
#ifdef INCLUDE_LIB_OPENEXR
#ifdef FREEIMAGE_SYSTEM_OPENEXR
#include <OpenEXR/OpenEXRConfig.h>
#include <Imath/ImathConfig.h>
#else
#include "../OpenEXR/OpenEXRConfig.h"
#include "../Imath/ImathConfig.h"
#endif
#endif
#ifdef FREEIMAGE_LIBHEIF
#include <libheif/heif.h>
#endif
#ifdef FREEIMAGE_LIBJXL
#include <jxl/decode.h>
#include <jxl/version.h>
#endif
#define FI_STRING2(v) #v
#define FI_STRING(v) FI_STRING2(v)
#ifdef FREEIMAGE_SYSTEM_ZLIB
#define Z_SOURCE TRUE
#else
#define Z_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_LIBPNG
#define PNG_SOURCE TRUE
#else
#define PNG_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_LIBJPEG
#define JPEG_SOURCE TRUE
#else
#define JPEG_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_LIBTIFF
#define TIFF_SOURCE TRUE
#else
#define TIFF_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_LIBOPENJPEG
#define OPJ_SOURCE TRUE
#else
#define OPJ_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_WEBP
#define WEBP_SOURCE TRUE
#else
#define WEBP_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_LIBRAWLITE
#define RAW_SOURCE TRUE
#else
#define RAW_SOURCE FALSE
#endif
#ifdef FREEIMAGE_SYSTEM_OPENEXR
#define EXR_SOURCE TRUE
#else
#define EXR_SOURCE FALSE
#endif
namespace {
struct Dependencies {
    FI_DEPENDENCY_INFO values[12]; unsigned count;
    std::string webp, jxl;
    Dependencies() : count(0) {
        Add("zlib", ZLIB_VERSION, zlibVersion(), Z_SOURCE);
        Add("libpng", PNG_LIBPNG_VER_STRING, png_get_libpng_ver(NULL), PNG_SOURCE);
        Add("libjpeg", "libjpeg API " FI_STRING(JPEG_LIB_VERSION), NULL, JPEG_SOURCE);
        Add("OpenJPEG", NULL, opj_version(), OPJ_SOURCE);
#if defined(INCLUDE_LIB_TIFF4) || defined(FREEIMAGE_SYSTEM_LIBTIFF)
        Add("libtiff", TIFFLIB_VERSION_STR, TIFFGetVersion(), TIFF_SOURCE);
#endif
#ifdef INCLUDE_LIB_WEBP
        const int webp_version=WebPGetDecoderVersion();
        webp=std::to_string(webp_version>>16)+"."+std::to_string((webp_version>>8)&255)+"."+std::to_string(webp_version&255);
        Add("WebP",NULL,webp.c_str(),WEBP_SOURCE);
#endif
#ifdef INCLUDE_LIB_RAW
        Add("LibRaw",LIBRAW_VERSION_STR,libraw_version(),RAW_SOURCE);
#endif
#ifdef INCLUDE_LIB_OPENEXR
        Add("OpenEXR",OPENEXR_VERSION_STRING,NULL,EXR_SOURCE);
        Add("Imath",IMATH_VERSION_STRING,NULL,EXR_SOURCE);
#endif
#ifdef INCLUDE_LIB_JXR
#ifdef FREEIMAGE_SYSTEM_JXR
        Add("jxrlib",NULL,NULL,TRUE);
#else
        Add("jxrlib","unversioned snapshot",NULL,FALSE);
#endif
#endif
#ifdef FREEIMAGE_LIBHEIF
        Add("libheif",LIBHEIF_VERSION,heif_get_version(),TRUE);
#endif
#ifdef FREEIMAGE_LIBJXL
        const unsigned version=JxlDecoderVersion();
        jxl=std::to_string(version/1000000)+"."+std::to_string((version/1000)%1000)+"."+std::to_string(version%1000);
        Add("libjxl",FI_STRING(JPEGXL_MAJOR_VERSION) "." FI_STRING(JPEGXL_MINOR_VERSION) "." FI_STRING(JPEGXL_PATCH_VERSION),jxl.c_str(),TRUE);
#endif
    }
    void Add(const char *name,const char *compiled,const char *runtime,BOOL system) {
        FI_DEPENDENCY_INFO info={name,compiled,runtime,system}; values[count++]=info;
    }
};
const Dependencies &Get() { static const Dependencies info; return info; }
}
unsigned DLL_CALLCONV FreeImage_GetDependenciesCount() { try { return Get().count; } catch(...) { return 0; } }
BOOL DLL_CALLCONV FreeImage_GetDependencyInfo(unsigned index, FI_DEPENDENCY_INFO *info) {
    try {
        if(!info || index>=Get().count) return FALSE;
        *info=Get().values[index]; return TRUE;
    } catch(...) { return FALSE; }
}
