#include "FreeImage.h"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

typedef std::vector<BYTE> Bytes;
typedef std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> Bitmap;

static void append32(Bytes &bytes, unsigned value) {
    for (int shift = 24; shift >= 0; shift -= 8)
        bytes.push_back(static_cast<BYTE>(value >> shift));
}

static Bytes makePbm(unsigned width, unsigned height, const Bytes &body) {
    Bytes chunks = {'B', 'M', 'H', 'D'};
    append32(chunks, 20);
    Bytes header(20, 0);
    header[0] = static_cast<BYTE>(width >> 8);
    header[1] = static_cast<BYTE>(width);
    header[2] = static_cast<BYTE>(height >> 8);
    header[3] = static_cast<BYTE>(height);
    header[8] = 8;
    header[10] = 1;
    chunks.insert(chunks.end(), header.begin(), header.end());
    const BYTE bodyId[] = {'B', 'O', 'D', 'Y'};
    chunks.insert(chunks.end(), bodyId, bodyId + sizeof(bodyId));
    append32(chunks, static_cast<unsigned>(body.size()));
    chunks.insert(chunks.end(), body.begin(), body.end());
    if (body.size() & 1)
        chunks.push_back(0);
    Bytes file = {'F', 'O', 'R', 'M'};
    append32(file, static_cast<unsigned>(chunks.size() + 4));
    const BYTE format[] = {'P', 'B', 'M', ' '};
    file.insert(file.end(), format, format + sizeof(format));
    file.insert(file.end(), chunks.begin(), chunks.end());
    return file;
}

static Bitmap load(Bytes &file) {
    FIMEMORY *stream = FreeImage_OpenMemory(file.data(), static_cast<DWORD>(file.size()));
    if (!stream)
        throw std::runtime_error("memory stream allocation failed");
    Bitmap image(FreeImage_LoadFromMemory(FIF_IFF, stream), &FreeImage_Unload);
    FreeImage_CloseMemory(stream);
    return image;
}

static void reject(const char *name, unsigned width, unsigned height, const Bytes &body) {
    Bytes file = makePbm(width, height, body);
    if (load(file))
        throw std::runtime_error(std::string(name) + ": truncated BODY was accepted");
}

static void rejectShortRead(const char *name, const Bytes &body, unsigned declaredSize) {
    Bytes file = makePbm(4, 1, body);
    if (body.size() & 1)
        file.pop_back();
    // Keep a BODY budget available when the underlying read actually hits EOF.
    for (unsigned i = 0; i < 4; ++i)
        file[44 + i] = static_cast<BYTE>(declaredSize >> (24 - 8 * i));
    if (load(file))
        throw std::runtime_error(std::string(name) + ": short read was accepted");
}

static void valid(const char *name, unsigned width, unsigned height, const Bytes &body,
                  const Bytes &pixels) {
    Bytes file = makePbm(width, height, body);
    Bitmap image = load(file);
    if (!image || FreeImage_GetWidth(image.get()) != width ||
        FreeImage_GetHeight(image.get()) != height)
        throw std::runtime_error(std::string(name) + ": valid image rejected");
    for (unsigned y = 0; y < height; ++y)
        if (std::memcmp(FreeImage_GetScanLine(image.get(), height - 1 - y),
                        pixels.data() + width * y, width))
            throw std::runtime_error(std::string(name) + ": pixels changed");
}

static void run(const std::string &name) {
    if (name == "empty")
        reject("empty", 4, 1, {});
    else if (name == "literal")
        reject("literal", 4, 1, {3, 17, 34});
    else if (name == "repeat")
        reject("repeat", 4, 1, {253});
    else if (name == "eof-command")
        rejectShortRead("eof-command", {}, 1);
    else if (name == "eof-literal")
        rejectShortRead("eof-literal", {3, 17, 34}, 5);
    else if (name == "eof-repeat")
        rejectShortRead("eof-repeat", {253}, 2);
    else if (name == "noops-eof")
        rejectShortRead("noops-eof", {128}, 2);
    else if (name == "noops")
        reject("noops", 4, 1, Bytes(1024, 128));
    else if (name == "row-noops")
        // #161 stops the oversized literal write, leaving these no-ops for row two.
        reject("row-noops", 4, 2, {8, 1, 2, 3, 4, 128, 128, 128, 128, 128});
    else if (name == "body-boundary") {
        Bytes file = makePbm(4, 1, {128});
        // Neither chunk padding nor bytes beyond BODY may complete the scanline.
        file.insert(file.end(), {253, 42});
        if (load(file))
            throw std::runtime_error("body-boundary: decoded beyond declared BODY");
    } else if (name == "valid-literal")
        valid("valid-literal", 3, 2, {128, 3, 1, 2, 3, 0, 128, 3, 4, 5, 6, 0},
              {1, 2, 3, 4, 5, 6});
    else if (name == "valid-repeat")
        valid("valid-repeat", 4, 2, {253, 21, 128, 253, 42},
              {21, 21, 21, 21, 42, 42, 42, 42});
    else
        throw std::runtime_error("unknown test case");
    std::printf("PBM RLE %s passed\n", name.c_str());
}

int main(int argc, char **argv) {
    FreeImage_Initialise();
    int result = 0;
    try {
        if (argc == 2)
            run(argv[1]);
        else
            for (const char *name : {"empty", "literal", "repeat", "eof-command", "eof-literal",
                                     "eof-repeat", "noops-eof", "noops", "row-noops",
                                     "body-boundary", "valid-literal", "valid-repeat"})
                run(name);
    } catch (const std::exception &error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        result = 1;
    }
    FreeImage_DeInitialise();
    return result;
}
