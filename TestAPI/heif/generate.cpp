// Generate synthetic colour/alpha fixtures; requires x265 and AOM encoders.
#include <cstdint>
#include <fstream>
typedef uint8_t BYTE;
#include <libheif/heif.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static void Require(bool ok, const char *what) {
    if(!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
static void Check(heif_error error) { Require(error.code == heif_error_Ok, error.message); }
static heif_error Write(heif_context *, const void *data, size_t size, void *opaque) {
    std::vector<BYTE> &bytes = *static_cast<std::vector<BYTE> *>(opaque);
    const BYTE *p = static_cast<const BYTE *>(data);
    bytes.insert(bytes.end(), p, p + size);
    heif_error result = {heif_error_Ok, heif_suberror_Unspecified, "OK"};
    return result;
}
static void Test(heif_compression_format codec, const char *filename, bool alpha) {
    heif_context *context = heif_context_alloc();
    Require(context != NULL, "context");
    heif_encoder *encoder = NULL;
    Check(heif_context_get_encoder_for_format(context, codec, &encoder));
    Check(heif_encoder_set_lossy_quality(encoder, 100));
    heif_image *image = NULL;
    Check(heif_image_create(32, 24, heif_colorspace_RGB,
        alpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB, &image));
    Check(heif_image_add_plane(image, heif_channel_interleaved, 32, 24, 8));
    int stride = 0;
    BYTE *pixels = heif_image_get_plane(image, heif_channel_interleaved, &stride);
    for(int y = 0; y < 24; ++y) for(int x = 0; x < 32; ++x) {
        BYTE *p = pixels + y * stride + x * (alpha ? 4 : 3);
        p[0] = y < 12 ? 220 : 30; p[1] = 70; p[2] = y < 12 ? 30 : 220;
        if(alpha) p[3] = y < 12 ? 255 : 100;
    }
    BYTE profile[128] = {};
    profile[3] = 128;
    std::memcpy(profile + 36, "acsp", 4);
    Check(heif_image_set_raw_color_profile(image, "prof", profile, sizeof(profile)));
    heif_image_handle *encoded = NULL;
    Check(heif_context_encode_image(context, image, encoder, NULL, &encoded));
    if(!std::strcmp(filename,"metadata.heic")) {
        const BYTE tiff[]={'I','I',42,0,8,0,0,0,1,0,0x12,1,3,0,1,0,0,0,1,0,0,0,0,0,0,0};
        Check(heif_context_add_exif_metadata(context,encoded,tiff,sizeof(tiff)));
        heif_image_handle *thumbnail=NULL;
        Check(heif_context_encode_thumbnail(context,image,encoded,encoder,NULL,16,&thumbnail));
        Require(thumbnail!=NULL,"thumbnail fixture"); heif_image_handle_release(thumbnail);
    }
    std::vector<BYTE> bytes;
    heif_writer writer = {}; writer.writer_api_version = 1; writer.write = Write;
    Check(heif_context_write(context, &writer, &bytes));
    heif_image_handle_release(encoded); heif_image_release(image);
    heif_encoder_release(encoder); heif_context_free(context);
    std::ofstream output(filename, std::ios::binary);
    output.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    Require(bool(output), "write fixture");
}
int main() {
    Test(heif_compression_HEVC, "metadata.heic", false);
    Test(heif_compression_HEVC, "rgb.heic", false);
    Test(heif_compression_HEVC, "rgba.heic", true);
    Test(heif_compression_AV1, "rgb.avif", false);
    Test(heif_compression_AV1, "rgba.avif", true);
}
