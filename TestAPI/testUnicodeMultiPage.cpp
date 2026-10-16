#include "FreeImage.h"
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <direct.h>
#endif

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "Failed line %d: %s\n", __LINE__, #x); std::exit(1); } } while (0)

static void roundTrip(bool memory) {
#ifdef _WIN32
    const wchar_t *path = L"multipage-\u65e5\u672c\u8a9e-\u00e9/image-\u4e2d\u6587.tif";
    _wmkdir(L"multipage-\u65e5\u672c\u8a9e-\u00e9");
    #define OPEN(create, read) FreeImage_OpenMultiBitmapU(FIF_TIFF, path, create, read, memory)
#else
    const char *path = "multipage-unicode-api-test.tif";
    #define OPEN(create, read) FreeImage_OpenMultiBitmap(FIF_TIFF, path, create, read, memory)
#endif
    FIBITMAP *page = FreeImage_Allocate(1024, 1024, 24);
    CHECK(page);
    // Incompressible content also exercises eviction from the disk page cache.
    unsigned state = 17;
    for (unsigned y = 0; y < 1024; ++y) {
        BYTE *row = FreeImage_GetScanLine(page, y);
        for (unsigned x = 0; x < 3072; ++x) {
            state = state * 1664525u + 1013904223u;
            row[x] = static_cast<BYTE>(state >> 24);
        }
    }
    const BYTE expected = FreeImage_GetScanLine(page, 0)[0];
    FIMULTIBITMAP *multi = OPEN(TRUE, FALSE);
    CHECK(multi);
    FreeImage_AppendPage(multi, page);
    FreeImage_AppendPage(multi, page);
    FreeImage_Unload(page);
    CHECK(FreeImage_CloseMultiBitmap(multi));
    multi = OPEN(FALSE, FALSE);
    CHECK(multi && FreeImage_GetPageCount(multi) == 2);
    page = FreeImage_LockPage(multi, 1);
    CHECK(page && FreeImage_GetScanLine(page, 0)[0] == expected);
    FreeImage_GetScanLine(page, 0)[0] = expected ^ 255;
    FreeImage_UnlockPage(multi, page, TRUE);
    CHECK(FreeImage_CloseMultiBitmap(multi));
    multi = OPEN(FALSE, TRUE);
    CHECK(multi && FreeImage_GetPageCount(multi) == 2);
    for (int i = 0; i < 2; ++i) {
        page = FreeImage_LockPage(multi, i);
        CHECK(page && FreeImage_GetScanLine(page, 0)[0] == (i ? (expected ^ 255) : expected));
        FreeImage_UnlockPage(multi, page, FALSE);
    }
    CHECK(FreeImage_CloseMultiBitmap(multi));
#ifdef _WIN32
    CHECK(_wremove(path) == 0);
    CHECK(_wrmdir(L"multipage-\u65e5\u672c\u8a9e-\u00e9") == 0); // cache/spool must be gone
#else
    CHECK(std::remove(path) == 0);
#endif
    #undef OPEN
}

int main() {
    FreeImage_Initialise();
    CHECK(!FreeImage_OpenMultiBitmapU(FIF_TIFF, NULL, TRUE, FALSE));
#ifndef _WIN32
    CHECK(!FreeImage_OpenMultiBitmapU(FIF_TIFF, L"unused.tif", TRUE, FALSE));
#endif
    roundTrip(false);
    roundTrip(true);
    FreeImage_DeInitialise();
    return 0;
}
