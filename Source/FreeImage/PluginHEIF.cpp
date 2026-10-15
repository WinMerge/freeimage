// ==========================================================
// HIEF and AVIF
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

// Decoder adapted from agruzdev/FreeImageRe, Source/Plugins/PluginHEIF.cpp
// revision b51d3945bfe785ad382a0969f1376aee344af8c6 (FIPL/GPLv2).
// Adaptation: classic plugin API, optional system linkage, checked stream
// callbacks, RGB/BGR conversion, and header-only loading. Encoding is omitted.
#include "FreeImage.h"
#include "Utilities.h"
#include <cstring>

static const char * DLL_CALLCONV HeifFormat() { return "HEIF"; }
static const char * DLL_CALLCONV AvifFormat() { return "AVIF"; }
static const char * DLL_CALLCONV HeifDescription() { return "HEIF image"; }
static const char * DLL_CALLCONV AvifDescription() { return "AVIF image"; }
static const char * DLL_CALLCONV HeifExtension() { return "heif,heic,heifs,heics"; }
static const char * DLL_CALLCONV AvifExtension() { return "avif"; }
static const char * DLL_CALLCONV HeifMime() { return "image/heif"; }
static const char * DLL_CALLCONV AvifMime() { return "image/avif"; }

#ifdef FREEIMAGE_LIBHEIF
#include <libheif/heif.h>
#include <climits>
#include <memory>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
struct Reader {
    FreeImageIO *io;
    fi_handle handle;
    long start, length;
    static int64_t Position(void *p) {
        Reader &r = *static_cast<Reader *>(p);
        const long position = r.io->tell_proc(r.handle);
        return position < r.start ? -1 : position - r.start;
    }
    static int Read(void *dst, size_t size, void *p) {
        Reader &r = *static_cast<Reader *>(p);
        const int64_t position = Position(p);
        if(position < 0 || position > r.length || size > static_cast<unsigned long>(r.length - position)) return 1;
        BYTE *out = static_cast<BYTE *>(dst);
        while(size) {
            const unsigned part = size > UINT_MAX ? UINT_MAX : static_cast<unsigned>(size);
            if(r.io->read_proc(out, 1, part, r.handle) != part) return 1;
            out += part;
            size -= part;
        }
        return 0;
    }
    static int Seek(int64_t position, void *p) {
        Reader &r = *static_cast<Reader *>(p);
        if(position < 0 || position > r.length) return 1;
        return r.io->seek_proc(r.handle, r.start + static_cast<long>(position), SEEK_SET);
    }
    static heif_reader_grow_status Wait(int64_t size, void *p) {
        Reader &r = *static_cast<Reader *>(p);
        return size >= 0 && size <= r.length ? heif_reader_grow_status_size_reached : heif_reader_grow_status_size_beyond_eof;
    }
};
void Check(heif_error error) {
    if(error.code != heif_error_Ok) throw std::runtime_error(error.message ? error.message : "libheif error");
}
BOOL Validate(FreeImageIO *io, fi_handle handle, bool avif) {
    const long start = io->tell_proc(handle);
    if(start < 0) return FALSE;
    BYTE bytes[128];
    const unsigned count = io->read_proc(bytes, 1, sizeof(bytes), handle);
    if(io->seek_proc(handle, start, SEEK_SET) != 0 || count < 12) return FALSE;
    const char *mime = heif_get_file_mime_type(bytes, static_cast<int>(count));
    return mime && (avif ? (!std::strcmp(mime, "image/avif") || !std::strcmp(mime, "image/avif-sequence")) :
        (!std::strcmp(mime, "image/heif") || !std::strcmp(mime, "image/heic") || !std::strcmp(mime, "image/heif-sequence") || !std::strcmp(mime, "image/heic-sequence")));
}
}
static BOOL DLL_CALLCONV ValidateHEIF(FreeImageIO *io, fi_handle handle) { return Validate(io, handle, false); }
static BOOL DLL_CALLCONV ValidateAVIF(FreeImageIO *io, fi_handle handle) { return Validate(io, handle, true); }
static BOOL DLL_CALLCONV SupportsNoPixels() { return TRUE; }
static BOOL DLL_CALLCONV SupportsICC() { return TRUE; }
template<FREE_IMAGE_FORMAT Format>
static FIBITMAP * DLL_CALLCONV Load(FreeImageIO *io, fi_handle handle, int, int flags, void *) {
    try {
        const long start = io->tell_proc(handle);
        if(start < 0 || io->seek_proc(handle, 0, SEEK_END) != 0) return NULL;
        const long end = io->tell_proc(handle);
        if(end < start || io->seek_proc(handle, start, SEEK_SET) != 0) return NULL;
        Reader stream = {io, handle, start, end - start};
        heif_reader reader = {};
        reader.reader_api_version = 1;
        reader.get_position = Reader::Position;
        reader.read = Reader::Read;
        reader.seek = Reader::Seek;
        reader.wait_for_file_size = Reader::Wait;
        std::unique_ptr<heif_context, void (*)(heif_context *)> context(heif_context_alloc(), heif_context_free);
        if(!context) return NULL;
        Check(heif_context_read_from_reader(context.get(), &reader, &stream, NULL));
        heif_image_handle *raw_handle = NULL;
        Check(heif_context_get_primary_image_handle(context.get(), &raw_handle));
        std::unique_ptr<heif_image_handle, void (*)(const heif_image_handle *)> image_handle(raw_handle, heif_image_handle_release);
        const int width = heif_image_handle_get_width(image_handle.get());
        const int height = heif_image_handle_get_height(image_handle.get());
        const bool alpha = heif_image_handle_has_alpha_channel(image_handle.get()) != 0;
        const int channels = alpha ? 4 : 3;
        if(width <= 0 || height <= 0 || width > INT_MAX / channels) return NULL;
        const BOOL header_only = (flags & FIF_LOAD_NOPIXELS) != 0;
        std::unique_ptr<FIBITMAP, void (*)(FIBITMAP *)> dib(
            FreeImage_AllocateHeaderT(header_only, FIT_BITMAP, width, height, channels * 8,
                FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK), FreeImage_Unload);
        if(!dib) return NULL;
        if(!header_only) {
            heif_image *raw_image = NULL;
            Check(heif_decode_image(image_handle.get(), &raw_image, heif_colorspace_RGB,
                alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB, NULL));
            std::unique_ptr<heif_image, void (*)(const heif_image *)> image(raw_image, heif_image_release);
            int stride = 0;
            const uint8_t *pixels = heif_image_get_plane_readonly(image.get(), heif_channel_interleaved, &stride);
            if(!pixels || stride < width * channels ||
                heif_image_get_width(image.get(), heif_channel_interleaved) != width ||
                heif_image_get_height(image.get(), heif_channel_interleaved) != height ||
                static_cast<size_t>(height) > std::numeric_limits<size_t>::max() / static_cast<size_t>(stride)) return NULL;
            for(int y = 0; y < height; ++y) {
                const uint8_t *src = pixels + static_cast<size_t>(y) * stride;
                BYTE *dst = FreeImage_GetScanLine(dib.get(), height - 1 - y);
                for(int x = 0; x < width; ++x, src += channels, dst += channels) {
                    dst[FI_RGBA_RED] = src[0];
                    dst[FI_RGBA_GREEN] = src[1];
                    dst[FI_RGBA_BLUE] = src[2];
                    if(alpha) dst[FI_RGBA_ALPHA] = src[3];
                }
            }
        }
        const size_t profile_size = heif_image_handle_get_raw_color_profile_size(image_handle.get());
        if(profile_size && profile_size <= static_cast<size_t>(LONG_MAX) && profile_size <= static_cast<size_t>(stream.length)) {
            std::vector<BYTE> profile(profile_size);
            Check(heif_image_handle_get_raw_color_profile(image_handle.get(), profile.data()));
            FIICCPROFILE *icc = FreeImage_CreateICCProfile(dib.get(), profile.data(), static_cast<long>(profile_size));
            if(!icc || !icc->data) return NULL;
        }
        return dib.release();
    } catch(const std::exception &error) {
        FreeImage_OutputMessageProc(Format, "%s", error.what());
        return NULL;
    }
}
#endif

static void Init(Plugin *plugin, bool avif) {
    std::memset(plugin, 0, sizeof(*plugin));
    plugin->format_proc = avif ? AvifFormat : HeifFormat;
    plugin->description_proc = avif ? AvifDescription : HeifDescription;
    plugin->extension_proc = avif ? AvifExtension : HeifExtension;
    plugin->mime_proc = avif ? AvifMime : HeifMime;
#ifdef FREEIMAGE_LIBHEIF
    plugin->validate_proc = avif ? ValidateAVIF : ValidateHEIF;
    plugin->load_proc = avif ? Load<FIF_AVIF> : Load<FIF_HEIF>;
    plugin->supports_no_pixels_proc = SupportsNoPixels;
    plugin->supports_icc_profiles_proc = SupportsICC;
#endif
}
void DLL_CALLCONV InitHEIF(Plugin *plugin, int) { Init(plugin, false); }
void DLL_CALLCONV InitAVIF(Plugin *plugin, int) { Init(plugin, true); }
