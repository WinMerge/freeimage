#include <FreeImage.h>
#include <stdio.h>
int main(void) {
    FreeImage_Initialise(FALSE);
    FIBITMAP *image = FreeImage_Allocate(2, 2, 24);
    RGBQUAD color = {0};
    color.rgbRed = 123;
    if (!image || !FreeImage_SetPixelColor(image, 1, 1, &color)) return 1;
    FIMEMORY *stream = FreeImage_OpenMemory(NULL, 0);
    if (!stream || !FreeImage_SaveToMemory(FIF_PNG, image, stream, 0)) return 2;
    FreeImage_SeekMemory(stream, 0, SEEK_SET);
    FIBITMAP *loaded = FreeImage_LoadFromMemory(FIF_PNG, stream, 0);
    RGBQUAD actual = {0};
    if (!loaded || !FreeImage_GetPixelColor(loaded, 1, 1, &actual) ||
        actual.rgbRed != color.rgbRed) return 3;
    printf("Installed FreeImage %s: PNG memory round trip passed\n", FreeImage_GetVersion());
    FreeImage_Unload(loaded);
    FreeImage_CloseMemory(stream);
    FreeImage_Unload(image);
    FreeImage_DeInitialise();
    return 0;
}
