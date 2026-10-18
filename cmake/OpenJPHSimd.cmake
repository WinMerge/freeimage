# OpenJPH's scalar and dispatch sources stay at the consumer's baseline ISA.
# Only the separately compiled x86 kernels receive ISA-specific options.
include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)

set(FREEIMAGE_OPENJPH_SIMD_ENABLED OFF)
if(FREEIMAGE_OPENJPH_SIMD)
    if(APPLE AND CMAKE_OSX_ARCHITECTURES)
        # A universal build must select the union of its source files. The
        # header guards leave these files empty for its non-x86 slices.
        if("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES OR "i386" IN_LIST CMAKE_OSX_ARCHITECTURES)
            set(FREEIMAGE_OPENJPH_TARGET_X86 ON)
        else()
            set(FREEIMAGE_OPENJPH_TARGET_X86 OFF)
        endif()
    else()
        # Processor/host names are unreliable for VS -A ARM64/ARM64EC and
        # compiler-driven cross builds. Inspect the actual compiler target.
        check_cxx_source_compiles("
#if defined(_M_ARM64EC) || !(defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64))
#error OpenJPH x86 SIMD is unavailable on this target
#endif
int main() { return 0; }
" FREEIMAGE_OPENJPH_TARGET_X86)
    endif()
    if(FREEIMAGE_OPENJPH_TARGET_X86)
        set(FREEIMAGE_OPENJPH_SIMD_ENABLED ON)
    endif()
endif()

set(_ojph_definitions)
if(FREEIMAGE_OPENJPH_SIMD_ENABLED)
    list(APPEND _ojph_definitions FREEIMAGE_OPENJPH_SIMD=1)

    set(_ojph_SSE_SOURCES codestream/ojph_codestream_sse.cpp
        transform/ojph_colour_sse.cpp transform/ojph_transform_sse.cpp)
    set(_ojph_SSE2_SOURCES codestream/ojph_codestream_sse2.cpp
        transform/ojph_colour_sse2.cpp transform/ojph_transform_sse2.cpp)
    set(_ojph_SSSE3_SOURCES coding/ojph_block_decoder_ssse3.cpp)
    set(_ojph_AVX_SOURCES codestream/ojph_codestream_avx.cpp
        transform/ojph_colour_avx.cpp transform/ojph_transform_avx.cpp)
    set(_ojph_AVX2_SOURCES codestream/ojph_codestream_avx2.cpp
        coding/ojph_block_decoder_avx2.cpp coding/ojph_block_encoder_avx2.cpp
        transform/ojph_colour_avx2.cpp transform/ojph_transform_avx2.cpp)
    set(_ojph_AVX512_SOURCES coding/ojph_block_encoder_avx512.cpp
        transform/ojph_transform_avx512.cpp)
    set(_ojph_SSE_FLAGS -msse)
    set(_ojph_SSE2_FLAGS -msse2)
    set(_ojph_SSSE3_FLAGS -mssse3)
    set(_ojph_AVX_FLAGS -mavx)
    set(_ojph_AVX2_FLAGS -mavx2)
    set(_ojph_AVX512_FLAGS -mavx512f -mavx512cd -mavx512bw -mavx512vl)

    foreach(_ojph_isa SSE SSE2 SSSE3 AVX AVX2 AVX512)
        set(_ojph_flags)
        if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            # Real MSVC allows SSE intrinsics at baseline, unlike clang-cl.
            if(_ojph_isa MATCHES "^AVX")
                set(_ojph_flags "/arch:${_ojph_isa}")
            endif()
        else()
            foreach(_ojph_flag IN LISTS _ojph_${_ojph_isa}_FLAGS)
                if(MSVC)
                    list(APPEND _ojph_flags "/clang:${_ojph_flag}")
                elseif(APPLE AND CMAKE_OSX_ARCHITECTURES)
                    foreach(_ojph_arch IN LISTS CMAKE_OSX_ARCHITECTURES)
                        if(_ojph_arch MATCHES "^(x86_64|i386)$")
                            list(APPEND _ojph_flags "-Xarch_${_ojph_arch}" "${_ojph_flag}")
                        endif()
                    endforeach()
                else()
                    list(APPEND _ojph_flags "${_ojph_flag}")
                endif()
            endforeach()
        endif()
        if(_ojph_flags)
            string(JOIN " " _ojph_probe_flags ${_ojph_flags})
            check_cxx_compiler_flag("${_ojph_probe_flags}" FREEIMAGE_OPENJPH_HAS_${_ojph_isa})
        else()
            set(FREEIMAGE_OPENJPH_HAS_${_ojph_isa} ON)
        endif()
        if(FREEIMAGE_OPENJPH_HAS_${_ojph_isa})
            foreach(_ojph_file IN LISTS _ojph_${_ojph_isa}_SOURCES)
                set(_ojph_source "${CMAKE_CURRENT_SOURCE_DIR}/Source/OpenEXR/OpenJPH/${_ojph_file}")
                list(APPEND FreeImage_OPENJPH_SRCS "${_ojph_source}")
                set_property(SOURCE "${_ojph_source}" APPEND PROPERTY COMPILE_OPTIONS ${_ojph_flags})
            endforeach()
        else()
            # Remove both the kernels and dispatch references when a compiler
            # lacks this ISA, preserving the slower implementations.
            list(APPEND _ojph_definitions "OJPH_DISABLE_${_ojph_isa}")
        endif()
    endforeach()
    if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # _xgetbv is used only after the runtime OSXSAVE check. clang-cl's
        # intrinsic needs the xsave feature on this CPU-probe source.
        # Match the source-list spelling: CMake keeps properties for
        # ./Source/... and Source/... separately with our policy baseline.
        set_property(SOURCE ./Source/OpenEXR/OpenJPH/others/ojph_arch.cpp
            APPEND PROPERTY COMPILE_OPTIONS /clang:-mxsave)
    endif()
endif()
if(_ojph_definitions)
    set_property(SOURCE ${FreeImage_OPENJPH_SRCS} APPEND PROPERTY COMPILE_DEFINITIONS ${_ojph_definitions})
endif()
message(STATUS "Bundled OpenJPH x86 SIMD: ${FREEIMAGE_OPENJPH_SIMD_ENABLED}")
