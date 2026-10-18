// JPEG XL still-image plugin. Adapted from the libjxl integration in
// agruzdev/FreeImageRe (b51d3945bfe785ad382a0969f1376aee344af8c6), FIPL/GPLv2.
// Uses the classic plugin API and system libjxl, without Plugin2/yato.
#include "FreeImage.h"
#include "Utilities.h"
#include <cstring>
static const char * DLL_CALLCONV Format() { return "JXL"; }
static const char * DLL_CALLCONV Description() { return "JPEG XL image"; }
static const char * DLL_CALLCONV Extension() { return "jxl"; }
static const char * DLL_CALLCONV Mime() { return "image/jxl"; }
#ifdef FREEIMAGE_LIBJXL
#include <jxl/decode.h>
#include <jxl/encode.h>
#include <jxl/color_encoding.h>
#include <jxl/version.h>
#if JPEGXL_NUMERIC_VERSION < JPEGXL_COMPUTE_NUMERIC_VERSION(0,7,0)
#error "FreeImage requires libjxl 0.7 or newer"
#endif
#include <climits>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
static void Require(bool ok) { if(!ok) throw std::runtime_error("Invalid or unsupported JPEG XL image"); }
static BOOL DLL_CALLCONV Yes() { return TRUE; }
static BOOL DLL_CALLCONV Depth(int bpp) { return bpp == 8 || bpp == 24 || bpp == 32; }
static BOOL DLL_CALLCONV Type(FREE_IMAGE_TYPE t) {
    return t == FIT_BITMAP || t == FIT_UINT16 || t == FIT_RGB16 || t == FIT_RGBA16 || t == FIT_FLOAT || t == FIT_RGBF || t == FIT_RGBAF;
}
static BOOL DLL_CALLCONV Validate(FreeImageIO *io, fi_handle h) {
    const long start = io->tell_proc(h); if(start < 0) return FALSE;
    BYTE bytes[12]; const unsigned size = io->read_proc(bytes,1,12,h);
    if(io->seek_proc(h,start,SEEK_SET)) return FALSE;
    const JxlSignature signature = JxlSignatureCheck(bytes,size);
    return signature == JXL_SIG_CODESTREAM || signature == JXL_SIG_CONTAINER;
}
static FIBITMAP * DLL_CALLCONV Load(FreeImageIO *io, fi_handle h, int, int flags, void *) {
    try {
        const long start=io->tell_proc(h); Require(start>=0 && !io->seek_proc(h,0,SEEK_END));
        const long end=io->tell_proc(h); Require(end>start && static_cast<unsigned long>(end-start)<=UINT_MAX && !io->seek_proc(h,start,SEEK_SET));
        std::vector<BYTE> input(end-start); Require(io->read_proc(input.data(),1,static_cast<unsigned>(input.size()),h)==input.size());
        std::unique_ptr<JxlDecoder,void(*)(JxlDecoder *)> decoder(JxlDecoderCreate(NULL),JxlDecoderDestroy); Require(bool(decoder));
        Require(JxlDecoderSubscribeEvents(decoder.get(),JXL_DEC_BASIC_INFO|JXL_DEC_COLOR_ENCODING|JXL_DEC_FULL_IMAGE)==JXL_DEC_SUCCESS);
        Require(JxlDecoderSetUnpremultiplyAlpha(decoder.get(),JXL_TRUE)==JXL_DEC_SUCCESS);
        Require(JxlDecoderSetInput(decoder.get(),input.data(),input.size())==JXL_DEC_SUCCESS); JxlDecoderCloseInput(decoder.get());
        JxlBasicInfo info={}; JxlPixelFormat format={};
        FREE_IMAGE_TYPE type=FIT_UNKNOWN; unsigned sample=0;
        std::unique_ptr<FIBITMAP,void(*)(FIBITMAP *)> dib(NULL,FreeImage_Unload);
        std::vector<BYTE> output;
        for(;;) {
            const JxlDecoderStatus status=JxlDecoderProcessInput(decoder.get());
            if(status==JXL_DEC_BASIC_INFO) {
                Require(JxlDecoderGetBasicInfo(decoder.get(),&info)==JXL_DEC_SUCCESS);
                Require((!info.exponent_bits_per_sample && info.bits_per_sample<=16) || (info.exponent_bits_per_sample && info.exponent_bits_per_sample<=8 && info.bits_per_sample<=32));
                Require(!info.have_animation && info.xsize && info.ysize && info.xsize<=INT_MAX && info.ysize<=INT_MAX && (info.num_color_channels==1 || info.num_color_channels==3));
                format.num_channels=info.alpha_bits ? 4 : info.num_color_channels;
                sample=info.exponent_bits_per_sample ? 4 : info.bits_per_sample>8 ? 2 : 1;
                format.data_type=sample==4 ? JXL_TYPE_FLOAT : sample==2 ? JXL_TYPE_UINT16 : JXL_TYPE_UINT8;
                format.endianness=JXL_NATIVE_ENDIAN; format.align=0;
                if(sample==1) type=FIT_BITMAP;
                else if(sample==2) type=format.num_channels==1 ? FIT_UINT16 : format.num_channels==3 ? FIT_RGB16 : FIT_RGBA16;
                else type=format.num_channels==1 ? FIT_FLOAT : format.num_channels==3 ? FIT_RGBF : FIT_RGBAF;
                Require(info.xsize<=INT_MAX/(sample*format.num_channels));
                dib.reset(FreeImage_AllocateHeaderT(flags & FIF_LOAD_NOPIXELS,type,info.xsize,info.ysize,sample*format.num_channels*8,FI_RGBA_RED_MASK,FI_RGBA_GREEN_MASK,FI_RGBA_BLUE_MASK)); Require(bool(dib));
                if(type==FIT_BITMAP && format.num_channels==1) for(unsigned i=0;i<256;++i) {
                    RGBQUAD *p=FreeImage_GetPalette(dib.get())+i; p->rgbRed=p->rgbGreen=p->rgbBlue=static_cast<BYTE>(i);
                }
            } else if(status==JXL_DEC_COLOR_ENCODING) {
                Require(bool(dib)); size_t size=0;
#if JPEGXL_NUMERIC_VERSION < JPEGXL_COMPUTE_NUMERIC_VERSION(0,9,0)
                const JxlDecoderStatus profile_status=JxlDecoderGetICCProfileSize(decoder.get(),&format,JXL_COLOR_PROFILE_TARGET_DATA,&size);
#else
                const JxlDecoderStatus profile_status=JxlDecoderGetICCProfileSize(decoder.get(),JXL_COLOR_PROFILE_TARGET_DATA,&size);
#endif
                if(profile_status==JXL_DEC_SUCCESS && size) {
                    Require(size<=LONG_MAX); std::vector<BYTE> profile(size);
#if JPEGXL_NUMERIC_VERSION < JPEGXL_COMPUTE_NUMERIC_VERSION(0,9,0)
                    Require(JxlDecoderGetColorAsICCProfile(decoder.get(),&format,JXL_COLOR_PROFILE_TARGET_DATA,profile.data(),size)==JXL_DEC_SUCCESS);
#else
                    Require(JxlDecoderGetColorAsICCProfile(decoder.get(),JXL_COLOR_PROFILE_TARGET_DATA,profile.data(),size)==JXL_DEC_SUCCESS);
#endif
                    FIICCPROFILE *icc=FreeImage_CreateICCProfile(dib.get(),profile.data(),static_cast<long>(size)); Require(icc && icc->data);
                }
                if(flags & FIF_LOAD_NOPIXELS) return dib.release();
            } else if(status==JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
                Require(bool(dib)); size_t size=0;
                Require(JxlDecoderImageOutBufferSize(decoder.get(),&format,&size)==JXL_DEC_SUCCESS);
                const size_t row=size_t(info.xsize)*sample*format.num_channels;
                Require(info.ysize<=std::numeric_limits<size_t>::max()/row && size==row*info.ysize);
                output.resize(size); Require(JxlDecoderSetImageOutBuffer(decoder.get(),&format,output.data(),size)==JXL_DEC_SUCCESS);
            } else if(status==JXL_DEC_FULL_IMAGE) {
                Require(bool(dib) && !output.empty()); const size_t row=size_t(info.xsize)*sample*format.num_channels;
                for(unsigned y=0;y<info.ysize;++y) {
                    BYTE *dst=FreeImage_GetScanLine(dib.get(),info.ysize-1-y); const BYTE *src=output.data()+y*row;
                    if(type==FIT_BITMAP && format.num_channels>=3) for(unsigned x=0;x<info.xsize;++x) {
                        dst[FI_RGBA_RED]=src[0]; dst[FI_RGBA_GREEN]=src[1]; dst[FI_RGBA_BLUE]=src[2];
                        if(format.num_channels==4) dst[FI_RGBA_ALPHA]=src[3]; dst+=format.num_channels; src+=format.num_channels;
                    } else std::memcpy(dst,src,row);
                }
                return dib.release();
            } else Require(false);
        }
    } catch(const std::exception &e) { FreeImage_OutputMessageProc(FIF_JPEGXL,"%s",e.what()); return NULL; }
}
static BOOL DLL_CALLCONV Save(FreeImageIO *io,FIBITMAP *dib,fi_handle h,int,int flags,void *) {
    try {
        Require(FreeImage_HasPixels(dib) && Type(FreeImage_GetImageType(dib)) && !(flags & ~(JXL_LOSSLESS|0x7f)) && (flags & 0x7f)<=100);
        std::unique_ptr<FIBITMAP,void(*)(FIBITMAP *)> converted(NULL,FreeImage_Unload);
        if(FreeImage_GetImageType(dib)==FIT_BITMAP && FreeImage_GetBPP(dib)==8) { converted.reset(FreeImage_ConvertTo24Bits(dib)); Require(bool(converted)); dib=converted.get(); }
        const FREE_IMAGE_TYPE type=FreeImage_GetImageType(dib); unsigned channels=0,sample=0;
        if(type==FIT_BITMAP) { Require(FreeImage_GetBPP(dib)==24 || FreeImage_GetBPP(dib)==32); channels=FreeImage_GetBPP(dib)/8; sample=1; }
        else { channels=(type==FIT_UINT16 || type==FIT_FLOAT) ? 1 : (type==FIT_RGB16 || type==FIT_RGBF) ? 3 : 4; sample=(type==FIT_UINT16 || type==FIT_RGB16 || type==FIT_RGBA16) ? 2 : 4; }
        JxlBasicInfo info; JxlEncoderInitBasicInfo(&info); info.xsize=FreeImage_GetWidth(dib); info.ysize=FreeImage_GetHeight(dib);
        info.bits_per_sample=sample*8; info.exponent_bits_per_sample=sample==4 ? 8 : 0; info.num_color_channels=channels==1 ? 1 : 3;
        info.num_extra_channels=channels==4 ? 1 : 0; info.alpha_bits=channels==4 ? sample*8 : 0; info.alpha_exponent_bits=channels==4 && sample==4 ? 8 : 0; info.uses_original_profile=JXL_TRUE;
        std::unique_ptr<JxlEncoder,void(*)(JxlEncoder *)> encoder(JxlEncoderCreate(NULL),JxlEncoderDestroy); Require(bool(encoder));
        Require(JxlEncoderSetBasicInfo(encoder.get(),&info)==JXL_ENC_SUCCESS);
        FIICCPROFILE *icc=FreeImage_GetICCProfile(dib);
        if(icc && icc->data && icc->size) Require(JxlEncoderSetICCProfile(encoder.get(),static_cast<const BYTE *>(icc->data),icc->size)==JXL_ENC_SUCCESS);
        else { JxlColorEncoding colour; if(sample==4) JxlColorEncodingSetToLinearSRGB(&colour,channels==1); else JxlColorEncodingSetToSRGB(&colour,channels==1); Require(JxlEncoderSetColorEncoding(encoder.get(),&colour)==JXL_ENC_SUCCESS); }
        JxlEncoderFrameSettings *settings=JxlEncoderFrameSettingsCreate(encoder.get(),NULL); Require(settings!=NULL);
        Require(JxlEncoderSetFrameLossless(settings,(flags & JXL_LOSSLESS) ? JXL_TRUE : JXL_FALSE)==JXL_ENC_SUCCESS);
        Require(JxlEncoderSetFrameDistance(settings,(flags & JXL_LOSSLESS) ? 0 : (100-((flags&0x7f) ? flags&0x7f : 90))/10.0f)==JXL_ENC_SUCCESS);
        const size_t row=size_t(info.xsize)*channels*sample; Require(row && info.ysize<=std::numeric_limits<size_t>::max()/row);
        std::vector<BYTE> pixels(row*info.ysize);
        for(unsigned y=0;y<info.ysize;++y) {
            const BYTE *src=FreeImage_GetScanLine(dib,info.ysize-1-y); BYTE *dst=pixels.data()+y*row;
            if(type==FIT_BITMAP) for(unsigned x=0;x<info.xsize;++x) {
                dst[0]=src[FI_RGBA_RED]; dst[1]=src[FI_RGBA_GREEN]; dst[2]=src[FI_RGBA_BLUE]; if(channels==4) dst[3]=src[FI_RGBA_ALPHA]; dst+=channels; src+=channels;
            } else std::memcpy(dst,src,row);
        }
        JxlPixelFormat format={channels,sample==4 ? JXL_TYPE_FLOAT : sample==2 ? JXL_TYPE_UINT16 : JXL_TYPE_UINT8,JXL_NATIVE_ENDIAN,0};
        Require(JxlEncoderAddImageFrame(settings,&format,pixels.data(),pixels.size())==JXL_ENC_SUCCESS); JxlEncoderCloseInput(encoder.get());
        for(;;) {
            BYTE buffer[65536]; BYTE *next=buffer; size_t remaining=sizeof(buffer);
            const JxlEncoderStatus status=JxlEncoderProcessOutput(encoder.get(),&next,&remaining);
            const unsigned size=static_cast<unsigned>(next-buffer); Require(io->write_proc(buffer,1,size,h)==size);
            if(status==JXL_ENC_SUCCESS) return TRUE; Require(status==JXL_ENC_NEED_MORE_OUTPUT);
        }
    } catch(const std::exception &e) { FreeImage_OutputMessageProc(FIF_JPEGXL,"%s",e.what()); return FALSE; }
}
#endif
void DLL_CALLCONV InitJPEGXL(Plugin *plugin,int) {
    std::memset(plugin,0,sizeof(*plugin)); plugin->format_proc=Format; plugin->description_proc=Description; plugin->extension_proc=Extension; plugin->mime_proc=Mime;
#ifdef FREEIMAGE_LIBJXL
    plugin->load_proc=Load; plugin->save_proc=Save; plugin->validate_proc=Validate; plugin->supports_export_bpp_proc=Depth;
    plugin->supports_export_type_proc=Type; plugin->supports_icc_profiles_proc=Yes; plugin->supports_no_pixels_proc=Yes;
#endif
}
