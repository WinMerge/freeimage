# OpenJPH x86 kernels for the GNU and macOS make builds. The parent makefile
# supplies its compiler and target flags so cross builds probe the target,
# rather than the machine running make. Override with FREEIMAGE_OPENJPH_SIMD=0.
FREEIMAGE_OPENJPH_SIMD ?= 1
ifneq ($(filter 1 ON on TRUE true,$(FREEIMAGE_OPENJPH_SIMD)),)
OPENJPH_SIMD_TARGET_X86 := $(shell $(OPENJPH_SIMD_CXX) $(OPENJPH_SIMD_CXXFLAGS) -dM -E -x c++ /dev/null 2>/dev/null | awk '/^\#define (__x86_64__|__i386__) / { print 1; exit }')
ifeq ($(OPENJPH_SIMD_TARGET_X86),1)
CXXFLAGS += -DFREEIMAGE_OPENJPH_SIMD=1
CPPFLAGS_X86_64 += -DFREEIMAGE_OPENJPH_SIMD=1
CPPFLAGS_I386 += -DFREEIMAGE_OPENJPH_SIMD=1

# Unsupported ISA flags remove both kernels and their dispatch references.
# Target-specific options leave scalar and CPU-detection files at baseline.
define openjph_add_isa
OPENJPH_HAS_$(1) := $$(shell $$(OPENJPH_SIMD_CXX) $$(OPENJPH_SIMD_CXXFLAGS) $(3) -c -x c++ /dev/null -o /dev/null >/dev/null 2>&1 && echo 1)
ifeq ($$(OPENJPH_HAS_$(1)),1)
SRCS += $(addprefix Source/OpenEXR/OpenJPH/,$(2))
$(patsubst %.cpp,%.o,$(addprefix Source/OpenEXR/OpenJPH/,$(2))): CXXFLAGS += $(3)
$(patsubst %.cpp,%.o-x86_64,$(addprefix Source/OpenEXR/OpenJPH/,$(2))): CPPFLAGS_X86_64 += $(3)
$(patsubst %.cpp,%.o-i386,$(addprefix Source/OpenEXR/OpenJPH/,$(2))): CPPFLAGS_I386 += $(3)
else
CXXFLAGS += -DOJPH_DISABLE_$(1)
CPPFLAGS_X86_64 += -DOJPH_DISABLE_$(1)
CPPFLAGS_I386 += -DOJPH_DISABLE_$(1)
endif
endef

$(eval $(call openjph_add_isa,SSE,codestream/ojph_codestream_sse.cpp transform/ojph_colour_sse.cpp transform/ojph_transform_sse.cpp,-msse))
$(eval $(call openjph_add_isa,SSE2,codestream/ojph_codestream_sse2.cpp transform/ojph_colour_sse2.cpp transform/ojph_transform_sse2.cpp,-msse2))
$(eval $(call openjph_add_isa,SSSE3,coding/ojph_block_decoder_ssse3.cpp,-mssse3))
$(eval $(call openjph_add_isa,AVX,codestream/ojph_codestream_avx.cpp transform/ojph_colour_avx.cpp transform/ojph_transform_avx.cpp,-mavx))
$(eval $(call openjph_add_isa,AVX2,codestream/ojph_codestream_avx2.cpp coding/ojph_block_decoder_avx2.cpp coding/ojph_block_encoder_avx2.cpp transform/ojph_colour_avx2.cpp transform/ojph_transform_avx2.cpp,-mavx2))
$(eval $(call openjph_add_isa,AVX512,coding/ojph_block_encoder_avx512.cpp transform/ojph_transform_avx512.cpp,-mavx512f -mavx512cd -mavx512bw -mavx512vl))
endif
endif
