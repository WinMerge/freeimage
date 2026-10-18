#include "FreeImage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
static void Check(bool v) { if(!v) std::exit(1); }
static const char * DLL_CALLCONV Format() { return "FAILSAVE"; }
static BOOL DLL_CALLCONV Fail(FreeImageIO *io, FIBITMAP *, fi_handle h, int, int, void *) {
    char partial[] = "incomplete"; io->write_proc(partial, 1, sizeof(partial), h); return FALSE;
}
static void DLL_CALLCONV Init(Plugin *p, int) { std::memset(p, 0, sizeof(*p)); p->format_proc = Format; p->save_proc = Fail; }
static std::string Read(const char *path) { std::ifstream f(path, std::ios::binary); return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()); }
int main() {
    FreeImage_Initialise();
    const char *path = "save-error-regression.png";
    FIBITMAP *dib = FreeImage_Allocate(8, 8, 24); Check(dib != NULL);
    { std::ofstream f(path); f << "original"; }
    const FREE_IMAGE_FORMAT fail = FreeImage_RegisterLocalPlugin(Init);
    Check(!FreeImage_Save(fail, dib, path)); Check(Read(path) == "original");
    Check(!FreeImage_Save(FIF_UNKNOWN, dib, path)); Check(Read(path) == "original");
    Check(!FreeImage_Save(FIF_PNG, NULL, path)); Check(Read(path) == "original");
    std::remove(path);
    Check(!FreeImage_Save(fail, dib, path)); Check(std::fopen(path, "rb") == NULL);
    Check(FreeImage_Save(FIF_PNG, dib, path));
    FIBITMAP *loaded = FreeImage_Load(FIF_PNG, path); Check(loaded && FreeImage_GetWidth(loaded) == 8);
    FreeImage_Unload(loaded); std::remove(path);
#ifdef _WIN32
    const wchar_t *wide = L"save-error-\x03b1.png";
    Check(FreeImage_SaveU(FIF_PNG, dib, wide));
    Check(!FreeImage_SaveU(fail, dib, wide));
    loaded = FreeImage_LoadU(FIF_PNG, wide); Check(loaded != NULL);
    FreeImage_Unload(loaded); _wremove(wide);
#endif
#ifndef _WIN32
    const std::string longname = std::string(251,'a') + ".png";
    Check(FreeImage_Save(FIF_PNG,dib,longname.c_str())); std::remove(longname.c_str());
    Check(!FreeImage_Save(FIF_PNG,dib,"/dev/null"));
#endif
    FreeImage_Unload(dib); FreeImage_DeInitialise();
    std::puts("Failed saves preserve destinations; successful replacement loads correctly");
}
