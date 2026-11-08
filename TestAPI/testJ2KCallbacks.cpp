// JPEG2000 callback regressions for issue #198. Checks also run in Release.
#include "FreeImage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void check(bool ok, const char *message) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

enum Fault {
    None, RejectEnd, RejectEndPositive, EndWithoutMoving,
    FailInitialTell, FailEndTell, RejectRestore, ZeroLength
};

struct Stream {
    FIMEMORY *memory;
    Fault fault;
    bool atEnd;
    unsigned reads;
};

static unsigned DLL_CALLCONV readStream(void *buffer, unsigned size, unsigned count,
                                       fi_handle handle) {
    Stream *stream = static_cast<Stream *>(handle);
    ++stream->reads;
    return FreeImage_ReadMemory(buffer, size, count, stream->memory);
}

static unsigned DLL_CALLCONV writeStream(void *buffer, unsigned size, unsigned count,
                                        fi_handle handle) {
    return FreeImage_WriteMemory(buffer, size, count, static_cast<Stream *>(handle)->memory);
}

static int DLL_CALLCONV seekStream(fi_handle handle, long offset, int origin) {
    Stream *stream = static_cast<Stream *>(handle);
    if (origin == SEEK_END && stream->fault == RejectEnd)
        return -1;
    if (origin == SEEK_END && stream->fault == RejectEndPositive)
        return 1;
    if (origin == SEEK_END && stream->fault == EndWithoutMoving)
        return 0;
    if (stream->atEnd && origin == SEEK_SET && stream->fault == RejectRestore)
        return -1;
    const BOOL ok = FreeImage_SeekMemory(stream->memory, offset, origin);
    if (ok)
        stream->atEnd = origin == SEEK_END;
    return ok ? 0 : -1;
}

static long DLL_CALLCONV tellStream(fi_handle handle) {
    Stream *stream = static_cast<Stream *>(handle);
    if (stream->fault == FailInitialTell || (stream->atEnd && stream->fault == FailEndTell))
        return -1;
    if (stream->atEnd && stream->fault == ZeroLength)
        return 0;
    return FreeImage_TellMemory(stream->memory);
}

static void testFormat(FREE_IMAGE_FORMAT format) {
    FreeImageIO io = {readStream, writeStream, seekStream, tellStream};
    FIBITMAP *image = FreeImage_Allocate(32, 32, 24);
    check(image != NULL, "Image allocation failed");
    for (unsigned y = 0; y < 32; ++y) {
        BYTE *row = FreeImage_GetScanLine(image, y);
        for (unsigned x = 0; x < 32 * 3; ++x)
            row[x] = static_cast<BYTE>(x + y);
    }
    FIMEMORY *memory = FreeImage_OpenMemory();
    check(memory != NULL, "Memory allocation failed");
    Stream output = {memory, RejectEnd, false, 0};
    check(FreeImage_SaveToHandle(format, image, &io, &output, 1) != FALSE,
          "Saving must not require SEEK_END on the output stream");
    const Fault faults[] = {None, RejectEnd, RejectEndPositive, EndWithoutMoving,
                            FailInitialTell, FailEndTell, RejectRestore, ZeroLength};
    for (unsigned i = 0; i < sizeof(faults) / sizeof(faults[0]); ++i) {
        for (unsigned headerOnly = 0; headerOnly < 2; ++headerOnly) {
            check(FreeImage_SeekMemory(memory, 0, SEEK_SET) != FALSE, "Memory rewind failed");
            // An unsupported SEEK_END does not affect signature detection.
            Stream input = {memory, faults[i] == RejectEnd ? RejectEnd : None, false, 0};
            check(FreeImage_GetFileTypeFromHandle(&io, &input, 0) == format,
                  "Handle format detection failed");
            check(FreeImage_TellMemory(memory) == 0, "Format detection moved the stream");
            input.fault = faults[i];
            input.reads = 0;
            FIBITMAP *loaded = FreeImage_LoadFromHandle(
                format, &io, &input, headerOnly ? FIF_LOAD_NOPIXELS : 0);
            if (faults[i] == None) {
                check(loaded != NULL, "Valid callbacks failed to load");
                check(FreeImage_GetWidth(loaded) == 32 && FreeImage_GetHeight(loaded) == 32,
                      "Loaded dimensions differ");
                check(FreeImage_GetImageType(loaded) == FIT_BITMAP && FreeImage_GetBPP(loaded) == 24,
                      "Loaded pixel type differs");
                check(!!FreeImage_HasPixels(loaded) == !headerOnly, "Header-only loading differs");
                if (!headerOnly) {
                    for (unsigned y = 0; y < 32; ++y)
                        check(std::memcmp(FreeImage_GetScanLine(loaded, y),
                                          FreeImage_GetScanLine(image, y), 32 * 3) == 0,
                              "Lossless round-trip pixels differ");
                }
            } else {
                check(loaded == NULL, "Invalid length callbacks must fail cleanly");
                check(input.reads == 0, "Decoder read data after a failed length probe");
                if (faults[i] != RejectRestore)
                    check(FreeImage_TellMemory(memory) == 0, "Failed probe moved the stream");
            }
            FreeImage_Unload(loaded);
        }
    }
    FreeImage_Unload(image);
    FreeImage_CloseMemory(memory);
    std::printf("%s callback tests passed\n", FreeImage_GetFormatFromFIF(format));
}

int main() {
    FreeImage_Initialise();
    testFormat(FIF_JP2);
    testFormat(FIF_J2K);
    FreeImage_DeInitialise();
    return 0;
}
