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
#if LIBHEIF_NUMERIC_VERSION < ((1 << 24) | (12 << 16))
#error "FreeImage requires libheif 1.12 or newer"
#endif
#include "../Metadata/FreeImageTag.h"
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
static FIBITMAP *DecodeImage(heif_image_handle *handle, int flags, long length) {
        const int width = heif_image_handle_get_width(handle);
        const int height = heif_image_handle_get_height(handle);
        const bool alpha = heif_image_handle_has_alpha_channel(handle) != 0;
        const int channels = alpha ? 4 : 3;
        if(width <= 0 || height <= 0 || width > INT_MAX / channels) return NULL;
        const BOOL header_only = (flags & FIF_LOAD_NOPIXELS) != 0;
        std::unique_ptr<FIBITMAP, void (*)(FIBITMAP *)> dib(
            FreeImage_AllocateHeaderT(header_only, FIT_BITMAP, width, height, channels * 8,
                FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK), FreeImage_Unload);
        if(!dib) return NULL;
        if(!header_only) {
            heif_image *raw_image = NULL;
            Check(heif_decode_image(handle, &raw_image, heif_colorspace_RGB,
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
        const size_t profile_size = heif_image_handle_get_raw_color_profile_size(handle);
        if(profile_size && profile_size <= static_cast<size_t>(LONG_MAX) && profile_size <= static_cast<size_t>(length)) {
            std::vector<BYTE> profile(profile_size);
            Check(heif_image_handle_get_raw_color_profile(handle, profile.data()));
            FIICCPROFILE *icc = FreeImage_CreateICCProfile(dib.get(), profile.data(), static_cast<long>(profile_size));
            if(!icc || !icc->data) return NULL;
        }
        return dib.release();
}
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
        std::unique_ptr<FIBITMAP, void (*)(FIBITMAP *)> dib(DecodeImage(image_handle.get(), flags, stream.length), FreeImage_Unload);
        if(!dib) return NULL;
        const int count = heif_image_handle_get_number_of_metadata_blocks(image_handle.get(), "Exif");
        if(count > 0 && count <= stream.length / 4) {
            std::vector<heif_item_id> ids(count);
            const int found = heif_image_handle_get_list_of_metadata_block_IDs(image_handle.get(), "Exif", ids.data(), count);
            for(int i = 0; i < found; ++i) {
                const size_t size = heif_image_handle_get_metadata_size(image_handle.get(), ids[i]);
                if(size <= 4 || size > static_cast<size_t>(stream.length) || size > UINT_MAX - 6) continue;
                std::vector<BYTE> metadata(size);
                Check(heif_image_handle_get_metadata(image_handle.get(), ids[i], metadata.data()));
                const size_t offset = (size_t(metadata[0]) << 24) | (size_t(metadata[1]) << 16) | (size_t(metadata[2]) << 8) | metadata[3];
                if(offset > size - 4) continue;
                std::vector<BYTE> exif(6 + size - 4 - offset);
                std::memcpy(exif.data(), "Exif\0\0", 6);
                std::memcpy(exif.data() + 6, metadata.data() + 4 + offset, size - 4 - offset);
                jpeg_read_exif_profile(dib.get(), exif.data(), static_cast<unsigned>(exif.size()));
                jpeg_read_exif_profile_raw(dib.get(), exif.data(), static_cast<unsigned>(exif.size()));
            }
        }
        if(!(flags & FIF_LOAD_NOPIXELS) && heif_image_handle_get_number_of_thumbnails(image_handle.get()) > 0) {
            heif_item_id id;
            if(heif_image_handle_get_list_of_thumbnail_IDs(image_handle.get(), &id, 1) == 1) {
                heif_image_handle *raw_thumbnail = NULL;
                Check(heif_image_handle_get_thumbnail(image_handle.get(), id, &raw_thumbnail));
                std::unique_ptr<heif_image_handle, void (*)(const heif_image_handle *)> thumbnail_handle(raw_thumbnail, heif_image_handle_release);
                std::unique_ptr<FIBITMAP, void (*)(FIBITMAP *)> thumbnail(DecodeImage(thumbnail_handle.get(), 0, stream.length), FreeImage_Unload);
                if(thumbnail) FreeImage_SetThumbnail(dib.get(), thumbnail.get());
            }
        }
        return dib.release();
    } catch(const std::exception &error) {
        FreeImage_OutputMessageProc(Format, "%s", error.what());
        return NULL;
    }
}
static heif_error WriteEncoded(heif_context *, const void *bytes, size_t size, void *opaque) {
    Reader &stream = *static_cast<Reader *>(opaque);
    const BYTE *p = static_cast<const BYTE *>(bytes);
    while(size) {
        const unsigned part = size > UINT_MAX ? UINT_MAX : static_cast<unsigned>(size);
        if(stream.io->write_proc(const_cast<BYTE *>(p), 1, part, stream.handle) != part) {
            heif_error error = {heif_error_Encoding_error, heif_suberror_Unspecified, "Short output write"}; return error;
        }
        p += part; size -= part;
    }
    heif_error ok = {heif_error_Ok, heif_suberror_Unspecified, "OK"}; return ok;
}
template<heif_compression_format Codec> static BOOL DLL_CALLCONV SupportsDepth(int bpp) {
    return (bpp == 24 || bpp == 32) && heif_have_encoder_for_format(Codec);
}
template<heif_compression_format Codec> static BOOL DLL_CALLCONV SupportsType(FREE_IMAGE_TYPE type) { return type == FIT_BITMAP && heif_have_encoder_for_format(Codec); }
template<heif_compression_format Codec, FREE_IMAGE_FORMAT Format>
static BOOL DLL_CALLCONV Save(FreeImageIO *io, FIBITMAP *dib, fi_handle output, int, int flags, void *) {
    try {
        const unsigned bpp = FreeImage_GetBPP(dib);
        const unsigned width = FreeImage_GetWidth(dib), height = FreeImage_GetHeight(dib);
        if(!FreeImage_HasPixels(dib) || FreeImage_GetImageType(dib) != FIT_BITMAP ||
            (bpp != 24 && bpp != 32) || width > INT_MAX || height > INT_MAX ||
            (flags & ~(HEIF_LOSSLESS | 0x7f)) || (flags & 0x7f) > 100) return FALSE;
        std::unique_ptr<heif_context, void (*)(heif_context *)> context(heif_context_alloc(), heif_context_free);
        if(!context) return FALSE;
        heif_encoder *raw_encoder = NULL;
        Check(heif_context_get_encoder_for_format(context.get(), Codec, &raw_encoder));
        std::unique_ptr<heif_encoder, void (*)(heif_encoder *)> encoder(raw_encoder, heif_encoder_release);
        Check(heif_encoder_set_lossy_quality(encoder.get(), (flags & 0x7f) ? flags & 0x7f : 90));
        if(flags & HEIF_LOSSLESS) Check(heif_encoder_set_lossless(encoder.get(), 1));
        heif_image *raw_image = NULL;
        Check(heif_image_create(width, height, heif_colorspace_RGB,
            bpp == 32 ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB, &raw_image));
        std::unique_ptr<heif_image, void (*)(const heif_image *)> image(raw_image, heif_image_release);
        Check(heif_image_add_plane(image.get(), heif_channel_interleaved, width, height, 8));
        int stride = 0;
        BYTE *pixels = heif_image_get_plane(image.get(), heif_channel_interleaved, &stride);
        const unsigned channels = bpp / 8;
        if(!pixels || width > INT_MAX / channels || stride < static_cast<int>(width * channels) || size_t(height)>std::numeric_limits<size_t>::max()/size_t(stride)) return FALSE;
        for(unsigned y = 0; y < height; ++y) {
            const BYTE *src = FreeImage_GetScanLine(dib, height - 1 - y);
            BYTE *dst = pixels + size_t(y) * stride;
            for(unsigned x = 0; x < width; ++x, src += channels, dst += channels) {
                dst[0] = src[FI_RGBA_RED]; dst[1] = src[FI_RGBA_GREEN]; dst[2] = src[FI_RGBA_BLUE];
                if(channels == 4) dst[3] = src[FI_RGBA_ALPHA];
            }
        }
        FIICCPROFILE *icc = FreeImage_GetICCProfile(dib);
        if(icc && icc->size && icc->data) Check(heif_image_set_raw_color_profile(image.get(), "prof", icc->data, icc->size));
        std::unique_ptr<heif_encoding_options, void (*)(heif_encoding_options *)> options(heif_encoding_options_alloc(), heif_encoding_options_free);
        std::unique_ptr<heif_color_profile_nclx, void (*)(heif_color_profile_nclx *)> colour(heif_nclx_color_profile_alloc(), heif_nclx_color_profile_free);
        if(!options || !colour) return FALSE;
        if(flags & HEIF_LOSSLESS) {
            Check(heif_encoder_set_parameter_string(encoder.get(), "chroma", "444"));
            colour->color_primaries = heif_color_primaries_ITU_R_BT_709_5;
            colour->transfer_characteristics = heif_transfer_characteristic_IEC_61966_2_1;
            colour->matrix_coefficients = heif_matrix_coefficients_RGB_GBR;
            colour->full_range_flag = 1;
            Check(heif_image_set_nclx_color_profile(image.get(), colour.get()));
            options->output_nclx_profile = colour.get();
            options->save_two_colr_boxes_when_ICC_and_nclx_available = 1;
        }
        heif_image_handle *raw_handle = NULL;
        Check(heif_context_encode_image(context.get(), image.get(), encoder.get(), options.get(), &raw_handle));
        std::unique_ptr<heif_image_handle, void (*)(const heif_image_handle *)> handle(raw_handle, heif_image_handle_release);
        FITAG *exif = NULL;
        if(FreeImage_GetMetadata(FIMD_EXIF_RAW, dib, "ExifRaw", &exif)) {
            const BYTE *data = static_cast<const BYTE *>(FreeImage_GetTagValue(exif));
            unsigned size = FreeImage_GetTagLength(exif);
            if(data && size > 6 && !std::memcmp(data, "Exif\0\0", 6)) {
                if(size > INT_MAX) return FALSE;
                data += 6; size -= 6;
                Check(heif_context_add_exif_metadata(context.get(), handle.get(), data, size));
            }
        }
        Reader stream = {io, output, 0, 0};
        heif_writer writer = {}; writer.writer_api_version = 1; writer.write = WriteEncoded;
        Check(heif_context_write(context.get(), &writer, &stream));
        return TRUE;
    } catch(const std::exception &error) { FreeImage_OutputMessageProc(Format, "%s", error.what()); return FALSE; }
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
    // Registration can run from FreeImage's shared-library initializer before
    // the bundled libheif C++ globals. Query encoders only from runtime calls.
    plugin->save_proc = avif ? Save<heif_compression_AV1, FIF_AVIF> : Save<heif_compression_HEVC, FIF_HEIF>;
    plugin->supports_export_bpp_proc = avif ? SupportsDepth<heif_compression_AV1> : SupportsDepth<heif_compression_HEVC>;
    plugin->supports_export_type_proc = avif ? SupportsType<heif_compression_AV1> : SupportsType<heif_compression_HEVC>;
    plugin->supports_no_pixels_proc = SupportsNoPixels;
    plugin->supports_icc_profiles_proc = SupportsICC;
#endif
}
void DLL_CALLCONV InitHEIF(Plugin *plugin, int) { Init(plugin, false); }
void DLL_CALLCONV InitAVIF(Plugin *plugin, int) { Init(plugin, true); }
