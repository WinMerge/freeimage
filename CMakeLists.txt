cmake_minimum_required(VERSION 3.16)

# Determine the version from the latest git tag (e.g. "3.19.21"), falling
# back to a static default when building from a source archive without
# .git history (or when git isn't installed). This has to run before
# project() below, since its VERSION argument needs a literal
# MAJOR.MINOR.PATCH at parse time - it can't be computed afterwards.
set(FREEIMAGE_VERSION_FALLBACK 3.19.21)
set(FREEIMAGE_VERSION_COMMITS 0)
set(FREEIMAGE_VERSION_HASH "unknown")
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --long --match "[0-9]*.[0-9]*.[0-9]*"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE FREEIMAGE_GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE FREEIMAGE_GIT_DESCRIBE_RESULT
    )
    if(FREEIMAGE_GIT_DESCRIBE_RESULT EQUAL 0)
        # e.g. "3.19.18-45-ge32af9e" -> tag 3.19.18, 45 commits past it, hash ge32af9e
        string(REGEX MATCH "^([0-9]+\\.[0-9]+\\.[0-9]+)-([0-9]+)-(g[0-9a-f]+)$" _fi_ver_match "${FREEIMAGE_GIT_DESCRIBE}")
        if(_fi_ver_match)
            set(FREEIMAGE_VERSION ${CMAKE_MATCH_1})
            set(FREEIMAGE_VERSION_COMMITS ${CMAKE_MATCH_2})
            set(FREEIMAGE_VERSION_HASH ${CMAKE_MATCH_3})
        endif()
    endif()
endif()
if(NOT FREEIMAGE_VERSION)
    set(FREEIMAGE_VERSION ${FREEIMAGE_VERSION_FALLBACK})
endif()

project(FreeImage VERSION ${FREEIMAGE_VERSION} LANGUAGES C CXX)

set(FREEIMAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(FREEIMAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(FREEIMAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Human-readable version string: bare "3.19.18" exactly on a tag, or
# "3.19.18+45.ge32af9e" (build metadata suffix, SemVer-style) in between.
if(FREEIMAGE_VERSION_COMMITS EQUAL 0)
    set(FREEIMAGE_VERSION_STRING "${FREEIMAGE_VERSION}")
else()
    set(FREEIMAGE_VERSION_STRING "${FREEIMAGE_VERSION}+${FREEIMAGE_VERSION_COMMITS}.${FREEIMAGE_VERSION_HASH}")
endif()
string(TIMESTAMP FREEIMAGE_COPYRIGHT_YEAR "%Y")
message(STATUS "FreeImage version: ${FREEIMAGE_VERSION_STRING}")

# Set C and C++ standards if provided. Applied early (before any option()
# calls below) so BUILD_OPENEXR's default, further down, can see whichever
# standard was actually requested.
if (DEFINED C_STANDARD)
    set(CMAKE_C_STANDARD ${C_STANDARD})
endif()
if (DEFINED CPP_STANDARD)
    set(CMAKE_CXX_STANDARD ${CPP_STANDARD})
endif()

# Options for configuring the build
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Whether the FreeImage target itself is built STATIC or SHARED, decoupled
# from the global BUILD_SHARED_LIBS: it only sets this option's *default*,
# so a project embedding FreeImage via add_subdirectory() can flip
# FREEIMAGE_STATIC independently of its own BUILD_SHARED_LIBS setting
# (which still governs any of *their* own add_library() calls with no
# explicit STATIC/SHARED). Mirrors libtiff's tiff-static option
# (https://gitlab.com/libtiff/libtiff/-/commit/e47dce81cd0190d124a08cbbb6f78a3e89d9511f).
if(BUILD_SHARED_LIBS)
    set(FREEIMAGE_STATIC_DEFAULT OFF)
else()
    set(FREEIMAGE_STATIC_DEFAULT ON)
endif()
option(FREEIMAGE_STATIC "Build FreeImage as a static library" ${FREEIMAGE_STATIC_DEFAULT})

# option() does not expand a bare WIN32 argument, so `option(... WIN32)`
# stayed OFF on Windows and CI never compiled jxrlib.
if(WIN32)
    set(BUILD_JXR_DEFAULT ON)
else()
    set(BUILD_JXR_DEFAULT OFF)
endif()
option(BUILD_JXR "Build JPEG-XR support" ${BUILD_JXR_DEFAULT})
option(BUILD_TESTS "Build the test suite" OFF)
option(FREEIMAGE_OPENJPH_SIMD "Enable runtime-dispatched x86 SIMD in bundled OpenJPH" ON)
option(BUILD_ZLIB "Build zlib from source or link" ON)
option(BUILD_LIBPNG "Build libpng from source or link" ON)
option(BUILD_LIBTIFF "Build libTIFF from source or link" ON)
option(BUILD_LIBJPEG "Build libjpeg from source or link" ON)
option(BUILD_LIBOPENJPEG "Build OpenJPEG from source or link" ON)
option(BUILD_WEBP "Build webp from source or link" ON)
# Camera RAW is part of the default build. -DBUILD_LIBRAWLITE=OFF leaves it out.
option(BUILD_LIBRAWLITE "Build librawlite from source or link" ON)
# Default ON, but only when C++17+ is actually in effect (OpenEXR needs it).
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD GREATER_EQUAL 17)
    set(BUILD_OPENEXR_DEFAULT ON)
else()
    set(BUILD_OPENEXR_DEFAULT OFF)
endif()
option(BUILD_OPENEXR "Build OpenEXR from source or link" ${BUILD_OPENEXR_DEFAULT})

# When ON, sets the default for every USE_SYSTEM_* option below to ON (each
# can still be overridden individually). Mirrors what MSYS2's
# mingw-w64-freeimage package does by unbundling all vendored sub-deps.
option(FREEIMAGE_USE_SYSTEM_LIBS "Link against system copies of vendored sub-dependencies instead of bundling them" OFF)
option(FREEIMAGE_LINUX_PACKAGE_LAYOUT "Install the Linux libfreeimage.so.3 package layout" OFF)
include(GNUInstallDirs)
if(FREEIMAGE_USE_SYSTEM_LIBS)
    set(FREEIMAGE_SYSTEM_DEFAULT ON)
else()
    set(FREEIMAGE_SYSTEM_DEFAULT OFF)
endif()

# Per-library toggles: only take effect when the matching BUILD_* option is
# ON. They select whether that dependency is compiled from the bundled
# Source/Lib* copy (OFF, default) or located on the system via
# find_package()/pkg-config (ON).
option(USE_SYSTEM_ZLIB "Link against system zlib instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_LIBPNG "Link against system libpng instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_LIBTIFF "Link against system libtiff instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_LIBJPEG "Link against system libjpeg instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_LIBOPENJPEG "Link against system openjpeg instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_WEBP "Link against system libwebp instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_LIBRAWLITE "Link against system libraw instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_OPENEXR "Link against system OpenEXR/Imath instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})
option(USE_SYSTEM_JXR "Link against system jxrlib instead of the bundled copy" ${FREEIMAGE_SYSTEM_DEFAULT})

# find_package(PNG)/find_package(ZLIB)'s actual include-dir hint variables
# (PNG_PNG_INCLUDE_DIR, ZLIB_INCLUDE_DIR) don't match the names older
# FreeImage.cmake consumers (e.g. openFrameworks' apothecary formula) pass
# for the same purpose (PNG_INCLUDE_DIR, ZLIB_INCLUDE_DIRS) - map them over
# so those older invocations keep working.
if(DEFINED PNG_INCLUDE_DIR AND NOT DEFINED PNG_PNG_INCLUDE_DIR)
    set(PNG_PNG_INCLUDE_DIR ${PNG_INCLUDE_DIR} CACHE PATH "Path to libpng include directory")
endif()
if(DEFINED ZLIB_INCLUDE_DIRS AND NOT DEFINED ZLIB_INCLUDE_DIR)
    set(ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIRS} CACHE PATH "Path to zlib include directory")
endif()

if(FREEIMAGE_STATIC)
    message(STATUS "Building FreeImage as a static library.")
else()
    message(STATUS "Building FreeImage as a shared library.")
endif()

#set(NO_BUILD_OPTIONS LIBRAWLITE LIBTIFF)

foreach(opt ${NO_BUILD_OPTIONS})
    if(NOT DEFINED NO_BUILD_${opt})
        set(NO_BUILD_${opt} OFF)
    endif()
    if(NO_BUILD_${opt})
        message("BUILD_${opt} = FALSE")
    else()
        set(BUILD_${opt} ON)
    endif()
endforeach()

set(FreeImage_SOURCES
    ./Source/FreeImage/BitmapAccess.cpp ./Source/FreeImage/ColorLookup.cpp
    ./Source/FreeImage/ConversionRGBA16.cpp ./Source/FreeImage/ConversionRGBAF.cpp
    ./Source/FreeImage/FreeImage.cpp ./Source/FreeImage/FreeImageC.c
    ./Source/FreeImage/FreeImageIO.cpp ./Source/FreeImage/GetType.cpp
    ./Source/FreeImage/LFPQuantizer.cpp ./Source/FreeImage/MemoryIO.cpp
    ./Source/FreeImage/PixelAccess.cpp ./Source/FreeImage/J2KHelper.cpp
    ./Source/FreeImage/MNGHelper.cpp ./Source/FreeImage/Plugin.cpp
    ./Source/FreeImage/PluginBMP.cpp ./Source/FreeImage/PluginCUT.cpp
    ./Source/FreeImage/PluginDDS.cpp ./Source/FreeImage/PluginEXR.cpp
    ./Source/FreeImage/PluginG3.cpp ./Source/FreeImage/PluginGIF.cpp
    ./Source/FreeImage/PluginHDR.cpp ./Source/FreeImage/PluginICO.cpp
    ./Source/FreeImage/PluginIFF.cpp ./Source/FreeImage/PluginJ2K.cpp
    ./Source/FreeImage/PluginJNG.cpp ./Source/FreeImage/PluginJP2.cpp
    ./Source/FreeImage/PluginJPEG.cpp ./Source/FreeImage/PluginJXR.cpp
    ./Source/FreeImage/PluginKOALA.cpp
    ./Source/FreeImage/PluginMNG.cpp ./Source/FreeImage/PluginPCD.cpp
    ./Source/FreeImage/PluginPCX.cpp ./Source/FreeImage/PluginPFM.cpp
    ./Source/FreeImage/PluginPICT.cpp ./Source/FreeImage/PluginPNG.cpp
    ./Source/FreeImage/PluginPNM.cpp ./Source/FreeImage/PluginPSD.cpp
    ./Source/FreeImage/PluginRAS.cpp ./Source/FreeImage/PluginRAW.cpp
    ./Source/FreeImage/PluginSGI.cpp ./Source/FreeImage/PluginTARGA.cpp
    ./Source/FreeImage/PluginTIFF.cpp ./Source/FreeImage/PluginWBMP.cpp
    ./Source/FreeImage/PluginWebP.cpp ./Source/FreeImage/PluginXBM.cpp
    ./Source/FreeImage/PluginXPM.cpp ./Source/FreeImage/PSDParser.cpp
    ./Source/FreeImage/TIFFLogLuv.cpp ./Source/FreeImage/Conversion.cpp
    ./Source/FreeImage/Conversion16_555.cpp ./Source/FreeImage/Conversion16_565.cpp
    ./Source/FreeImage/Conversion24.cpp ./Source/FreeImage/Conversion32.cpp
    ./Source/FreeImage/Conversion4.cpp ./Source/FreeImage/Conversion8.cpp
    ./Source/FreeImage/ConversionFloat.cpp ./Source/FreeImage/ConversionRGB16.cpp
    ./Source/FreeImage/ConversionRGBF.cpp ./Source/FreeImage/ConversionType.cpp
    ./Source/FreeImage/ConversionUINT16.cpp ./Source/FreeImage/Halftoning.cpp
    ./Source/FreeImage/tmoColorConvert.cpp ./Source/FreeImage/tmoDrago03.cpp
    ./Source/FreeImage/tmoFattal02.cpp ./Source/FreeImage/tmoReinhard05.cpp
    ./Source/FreeImage/ToneMapping.cpp ./Source/FreeImage/NNQuantizer.cpp
    ./Source/FreeImage/WuQuantizer.cpp ./Source/FreeImage/CacheFile.cpp
    ./Source/FreeImage/MultiPage.cpp ./Source/FreeImage/ZLibInterface.cpp
    ./Source/Metadata/Exif.cpp ./Source/Metadata/FIRational.cpp
    ./Source/Metadata/FreeImageTag.cpp ./Source/Metadata/IPTC.cpp
    ./Source/Metadata/TagConversion.cpp ./Source/Metadata/TagLib.cpp
    ./Source/Metadata/XTIFF.cpp ./Source/FreeImageToolkit/Background.cpp
    ./Source/FreeImageToolkit/BSplineRotate.cpp ./Source/FreeImageToolkit/Channels.cpp
    ./Source/FreeImageToolkit/ClassicRotate.cpp ./Source/FreeImageToolkit/Colors.cpp
    ./Source/FreeImageToolkit/CopyPaste.cpp ./Source/FreeImageToolkit/Display.cpp
    ./Source/FreeImageToolkit/Flip.cpp ./Source/FreeImageToolkit/JPEGTransform.cpp
    ./Source/FreeImageToolkit/MultigridPoissonSolver.cpp ./Source/FreeImageToolkit/Rescale.cpp
    ./Source/FreeImageToolkit/Resize.cpp
    ./Source/CacheFile.h ./Source/FreeImage ./Source/FreeImage.h
    ./Source/FreeImageIO.h ./Source/MapIntrospector.h
    ./Source/Metadata/FIRational.h ./Source/Metadata/FreeImageTag.h ./Source/Plugin.h
    ./Source/Quantizers.h ./Source/ToneMapping.h ./Source/Utilities.h ./TestAPI/TestSuite.h
)

if(BUILD_LIBJPEG)
    if(USE_SYSTEM_LIBJPEG)
        find_package(JPEG REQUIRED)
        list(APPEND FreeImage_LIBS JPEG::JPEG)
        add_definitions(-DFREEIMAGE_SYSTEM_LIBJPEG)
    else()
        set(FreeImage_LIBJPEG_SRCS
            Source/LibJPEG/jaricom.c Source/LibJPEG/jcapimin.c
            Source/LibJPEG/jcapistd.c Source/LibJPEG/jcarith.c Source/LibJPEG/jccoefct.c
            Source/LibJPEG/jccolor.c Source/LibJPEG/jcdctmgr.c Source/LibJPEG/jchuff.c
            Source/LibJPEG/jcinit.c Source/LibJPEG/jcmainct.c Source/LibJPEG/jcmarker.c
            Source/LibJPEG/jcmaster.c Source/LibJPEG/jcomapi.c Source/LibJPEG/jcparam.c
            Source/LibJPEG/jcprepct.c Source/LibJPEG/jcsample.c Source/LibJPEG/jctrans.c
            Source/LibJPEG/jdapimin.c Source/LibJPEG/jdapistd.c Source/LibJPEG/jdarith.c
            Source/LibJPEG/jdatadst.c Source/LibJPEG/jdatasrc.c Source/LibJPEG/jdcoefct.c
            Source/LibJPEG/jdcolor.c Source/LibJPEG/jddctmgr.c Source/LibJPEG/jdhuff.c
            Source/LibJPEG/jdinput.c Source/LibJPEG/jdmainct.c Source/LibJPEG/jdmarker.c
            Source/LibJPEG/jdmaster.c Source/LibJPEG/jdmerge.c Source/LibJPEG/jdpostct.c
            Source/LibJPEG/jdsample.c Source/LibJPEG/jdtrans.c Source/LibJPEG/jerror.c
            Source/LibJPEG/jfdctflt.c Source/LibJPEG/jfdctfst.c Source/LibJPEG/jfdctint.c
            Source/LibJPEG/jidctflt.c Source/LibJPEG/jidctfst.c Source/LibJPEG/jidctint.c
            Source/LibJPEG/jmemmgr.c Source/LibJPEG/jmemnobs.c Source/LibJPEG/jquant1.c
            Source/LibJPEG/jquant2.c Source/LibJPEG/jutils.c Source/LibJPEG/transupp.c
            ./Source/LibJPEG/cderror.h ./Source/LibJPEG/cdjpeg.h
            ./Source/LibJPEG/jconfig.h ./Source/LibJPEG/jdct.h ./Source/LibJPEG/jerror.h
            ./Source/LibJPEG/jinclude.h ./Source/LibJPEG/jpegint.h ./Source/LibJPEG/jpeglib.h
            ./Source/LibJPEG/jmemsys.h ./Source/LibJPEG/jmorecfg.h ./Source/LibJPEG/transupp.h
            ./Source/LibJPEG/jversion.h
        )
        list(APPEND FreeImage_SOURCES ${FreeImage_LIBJPEG_SRCS})
        include_directories(Source/LibJPEG)
    endif()
endif()

if(BUILD_LIBOPENJPEG)
    if(USE_SYSTEM_LIBOPENJPEG)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(OPENJPEG REQUIRED IMPORTED_TARGET libopenjp2)
        list(APPEND FreeImage_LIBS PkgConfig::OPENJPEG)
        add_definitions(-DFREEIMAGE_SYSTEM_LIBOPENJPEG)
    else()
        set(FreeImage_LIBOPENJPEG_SRCS
            Source/LibOpenJPEG/bio.c Source/LibOpenJPEG/cio.c Source/LibOpenJPEG/dwt.c
            Source/LibOpenJPEG/event.c Source/LibOpenJPEG/function_list.c Source/LibOpenJPEG/ht_dec.c
            Source/LibOpenJPEG/image.c Source/LibOpenJPEG/invert.c Source/LibOpenJPEG/j2k.c
            Source/LibOpenJPEG/jp2.c Source/LibOpenJPEG/mct.c Source/LibOpenJPEG/mqc.c
            Source/LibOpenJPEG/openjpeg.c Source/LibOpenJPEG/opj_clock.c Source/LibOpenJPEG/opj_malloc.c
            Source/LibOpenJPEG/pi.c Source/LibOpenJPEG/sparse_array.c Source/LibOpenJPEG/t1.c
            Source/LibOpenJPEG/t2.c Source/LibOpenJPEG/tcd.c Source/LibOpenJPEG/tgt.c
            Source/LibOpenJPEG/thread.c
        )
        list(APPEND FreeImage_SOURCES ${FreeImage_LIBOPENJPEG_SRCS})
        include_directories(Source/LibOpenJPEG)
    endif()
endif()

    # DEFINED ZLIB_LIBRARY / DEFINED PNG_LIBRARY: back-compat with older
    # FreeImage.cmake consumers (e.g. openFrameworks' apothecary formula)
    # that used -DBUILD_ZLIB=OFF/-DBUILD_LIBPNG=OFF themselves (not just
    # USE_SYSTEM_ZLIB/USE_SYSTEM_LIBPNG) to mean "link a system lib"
    # rather than "no support at all".
    #
    # DEFINED alone isn't enough: find_package(PNG) internally calls
    # find_package(ZLIB) to resolve libpng's own zlib dependency, and if
    # that inner search fails it still sets the ZLIB_LIBRARY *cache*
    # variable to ZLIB_LIBRARY-NOTFOUND - which DEFINED reports as true.
    # A caller passing only -DPNG_LIBRARY=... (no zlib info at all,
    # intending the bundled zlib below to satisfy libpng's dependency)
    # would then wrongly hit the system-zlib branch and fail with a
    # confusing "Could NOT find ZLIB" from our own find_package(ZLIB
    # REQUIRED), instead of what should happen: bundle Source/ZLib and
    # simply not use it as a global CMake target that a chosen PNG_LIBRARY
    # might already have its own idea of zlib. So: treat NOTFOUND the same
    # as unset, and resolve zlib before libpng, so that if a real
    # ZLIB_LIBRARY *was* provided (or is found on the system), FindPNG's
    # internal search reuses that same result instead of re-probing and
    # potentially finding something else (or nothing).
if(PNG_LIBRARY MATCHES "NOTFOUND")
    unset(PNG_LIBRARY CACHE)
endif()
if(ZLIB_LIBRARY MATCHES "NOTFOUND")
    unset(ZLIB_LIBRARY CACHE)
endif()

if(BUILD_ZLIB OR DEFINED ZLIB_LIBRARY)
    if(USE_SYSTEM_ZLIB OR DEFINED ZLIB_LIBRARY)
        find_package(ZLIB REQUIRED)
        list(APPEND FreeImage_LIBS ZLIB::ZLIB)
        add_definitions(-DFREEIMAGE_SYSTEM_ZLIB)
    else()
    set(FreeImage_ZLIB_SRCS
        Source/ZLib/adler32.c Source/ZLib/compress.c Source/ZLib/crc32.c Source/ZLib/deflate.c
        Source/ZLib/gzclose.c Source/ZLib/gzlib.c Source/ZLib/gzread.c Source/ZLib/gzwrite.c
        Source/ZLib/infback.c Source/ZLib/inffast.c Source/ZLib/inflate.c Source/ZLib/inftrees.c
        Source/ZLib/trees.c Source/ZLib/uncompr.c Source/ZLib/zutil.c
        ./Source/ZLib/crc32.h ./Source/ZLib/deflate.h ./Source/ZLib/gzguts.h
        ./Source/ZLib/inffast.h ./Source/ZLib/inffixed.h ./Source/ZLib/inflate.h
        ./Source/ZLib/inftrees.h ./Source/ZLib/trees.h ./Source/ZLib/zconf.h
        ./Source/ZLib/zlib.h ./Source/ZLib/zutil.h
    )
    list(APPEND FreeImage_SOURCES ${FreeImage_ZLIB_SRCS})
    include_directories(Source/ZLib)
    add_definitions(-DINCLUDE_LIB_ZLIB)
    if(NOT WIN32)
        # zlib >= 1.3 needs HAVE_UNISTD_H defined (normally set by zlib's own
        # configure/CMake auto-detection) for gzread.c/gzwrite.c to see
        # read()/write()/close() via zconf.h's Z_HAVE_UNISTD_H gate; we build
        # its sources directly rather than through that build system.
        add_definitions(-DHAVE_UNISTD_H)
    endif()
    endif()
endif()

if(BUILD_LIBPNG OR DEFINED PNG_LIBRARY)
    if(USE_SYSTEM_LIBPNG OR DEFINED PNG_LIBRARY)
        find_package(PNG REQUIRED)
        if(NOT TARGET PNG::PNG)
            # FindPNG.cmake only creates the PNG::PNG target if it could also
            # resolve zlib as a real target - it can silently report
            # PNG_FOUND=TRUE from PNG_LIBRARY/PNG_PNG_INCLUDE_DIR alone while
            # never creating PNG::PNG, which fails later with a much more
            # confusing "target PNG::PNG was not found" generate-time error.
            # A prebuilt PNG_LIBRARY needs a matching prebuilt zlib it was
            # actually linked against - bundling a separately-compiled zlib
            # here can't safely stand in for that, so require the caller to
            # supply one (ZLIB_LIBRARY=..., or ensure system zlib is
            # discoverable) rather than fail confusingly.
            message(FATAL_ERROR "PNG_LIBRARY was given (${PNG_LIBRARY}) but "
                "no zlib could be resolved for it to link against - pass "
                "-DZLIB_LIBRARY=<path> (matching what PNG_LIBRARY was built "
                "against) alongside -DPNG_LIBRARY, or ensure a system zlib "
                "is discoverable.")
        endif()
        list(APPEND FreeImage_LIBS PNG::PNG)
        add_definitions(-DFREEIMAGE_SYSTEM_LIBPNG)
    else()
        set(FreeImage_LIBPNG_SRCS
            Source/LibPNG/png.c Source/LibPNG/pngerror.c Source/LibPNG/pngget.c
            Source/LibPNG/pngmem.c Source/LibPNG/pngpread.c Source/LibPNG/pngread.c
            Source/LibPNG/pngrio.c Source/LibPNG/pngrtran.c Source/LibPNG/pngrutil.c
            Source/LibPNG/pngset.c Source/LibPNG/pngtrans.c Source/LibPNG/pngwio.c
            Source/LibPNG/pngwrite.c Source/LibPNG/pngwtran.c Source/LibPNG/pngwutil.c
            ./Source/LibPNG/png.h ./Source/LibPNG/pngconf.h ./Source/LibPNG/pngdebug.h
            ./Source/LibPNG/pnginfo.h ./Source/LibPNG/pnglibconf.h ./Source/LibPNG/pngpriv.h
            ./Source/LibPNG/pngstruct.h
        )
        list(APPEND FreeImage_SOURCES ${FreeImage_LIBPNG_SRCS})
        include_directories(Source/LibPNG)
        # pngpriv.h auto-enables ARM NEON codepaths (pngrtran.c/pngrutil.c)
        # whenever the compiler defines __ARM_NEON, referencing functions
        # from arm/*.c - which isn't part of this bundled, non-SIMD source
        # set, so force it off. The other arch accel paths (MIPS/PowerPC/
        # LoongArch) only auto-enable on those targets, which this project
        # doesn't build for, so they're not an issue here.
        add_definitions(-DPNG_ARM_NEON_OPT=0)
    endif()
endif()

if(BUILD_LIBTIFF)
    if(USE_SYSTEM_LIBTIFF)
        find_package(TIFF REQUIRED)
        list(APPEND FreeImage_LIBS TIFF::TIFF)
        add_definitions(-DFREEIMAGE_SYSTEM_LIBTIFF)
    else()
    set(FreeImage_LIBTIFF_SRCS
        Source/LibTIFF4/tif_error.c Source/LibTIFF4/tif_aux.c
        Source/LibTIFF4/tif_close.c Source/LibTIFF4/tif_codec.c Source/LibTIFF4/tif_color.c
        Source/LibTIFF4/tif_compress.c Source/LibTIFF4/tif_config.h
        Source/LibTIFF4/tif_config.vc.h Source/LibTIFF4/tif_config.wince.h
        Source/LibTIFF4/tif_dir.c Source/LibTIFF4/tif_dir.h Source/LibTIFF4/tif_dirinfo.c
        Source/LibTIFF4/tif_dirread.c Source/LibTIFF4/tif_dirwrite.c Source/LibTIFF4/tif_dumpmode.c
        Source/LibTIFF4/tif_error.c Source/LibTIFF4/tif_extension.c Source/LibTIFF4/tif_fax3.c
        Source/LibTIFF4/tif_fax3.h Source/LibTIFF4/tif_fax3sm.c Source/LibTIFF4/tif_flush.c
        Source/LibTIFF4/tif_getimage.c Source/LibTIFF4/tif_hash_set.c Source/LibTIFF4/tif_hash_set.h
        Source/LibTIFF4/tif_jbig.c Source/LibTIFF4/tif_jpeg.c Source/LibTIFF4/tif_jpeg_12.c
        Source/LibTIFF4/tif_lerc.c Source/LibTIFF4/tif_luv.c Source/LibTIFF4/tif_lzw.c
        Source/LibTIFF4/tif_next.c Source/LibTIFF4/tif_ojpeg.c Source/LibTIFF4/tif_open.c
        Source/LibTIFF4/tif_packbits.c Source/LibTIFF4/tif_pixarlog.c Source/LibTIFF4/tif_predict.c
        Source/LibTIFF4/tif_predict.h Source/LibTIFF4/tif_print.c Source/LibTIFF4/tif_read.c
        Source/LibTIFF4/tif_strip.c Source/LibTIFF4/tif_swab.c Source/LibTIFF4/tif_thunder.c
        Source/LibTIFF4/tif_tile.c Source/LibTIFF4/tif_version.c Source/LibTIFF4/tif_warning.c
        Source/LibTIFF4/tif_webp.c Source/LibTIFF4/tif_write.c Source/LibTIFF4/tif_zip.c
        Source/LibTIFF4/tif_zstd.c Source/LibTIFF4/tiff.h Source/LibTIFF4/tiffconf.h
        Source/LibTIFF4/tiffconf.vc.h Source/LibTIFF4/tiffconf.wince.h Source/LibTIFF4/tiffio.h
        Source/LibTIFF4/tiffiop.h Source/LibTIFF4/tiffvers.h Source/LibTIFF4/uvcode.h
        Source/LibTIFF4/t4.h Source/LibTIFF4/libport.h
    )
    if(WIN32)
        list(APPEND FreeImage_LIBTIFF_SRCS Source/LibTIFF4/tif_win32.c)
    endif()
    list(APPEND FreeImage_SOURCES ${FreeImage_LIBTIFF_SRCS})
    include_directories(Source/LibTIFF4)
    add_definitions(-DINCLUDE_LIB_TIFF4)
    endif()
endif()

if(BUILD_JXR)
    if(USE_SYSTEM_JXR)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(JXR REQUIRED IMPORTED_TARGET libjxr)
        list(APPEND FreeImage_LIBS PkgConfig::JXR)
        add_definitions(-DFREEIMAGE_SYSTEM_JXR -DINCLUDE_LIB_JXR)
    else()
    # Keep this list aligned with the LibJXR entries in Makefile.srcs.
    # The previous list stopped at "Source/LibJXR/image/" and omitted the
    # glue objects, so PluginJXR.cpp linked with 27 unresolved symbols.
    set(FreeImage_JXR_SRCS
        Source/LibJXR/image/decode/decode.c
        Source/LibJXR/image/decode/JXRTranscode.c
        Source/LibJXR/image/decode/postprocess.c
        Source/LibJXR/image/decode/segdec.c
        Source/LibJXR/image/decode/strdec.c
        Source/LibJXR/image/decode/strdec_x86.c
        Source/LibJXR/image/decode/strInvTransform.c
        Source/LibJXR/image/decode/strPredQuantDec.c
        Source/LibJXR/image/encode/encode.c
        Source/LibJXR/image/encode/segenc.c
        Source/LibJXR/image/encode/strenc.c
        Source/LibJXR/image/encode/strenc_x86.c
        Source/LibJXR/image/encode/strFwdTransform.c
        Source/LibJXR/image/encode/strPredQuantEnc.c
        Source/LibJXR/image/sys/adapthuff.c
        Source/LibJXR/image/sys/image.c
        Source/LibJXR/image/sys/strcodec.c
        Source/LibJXR/image/sys/strPredQuant.c
        Source/LibJXR/image/sys/strTransform.c
        Source/LibJXR/jxrgluelib/JXRGlue.c
        Source/LibJXR/jxrgluelib/JXRGlueJxr.c
        Source/LibJXR/jxrgluelib/JXRGluePFC.c
        Source/LibJXR/jxrgluelib/JXRMeta.c
        Source/LibJXR/common/include/guiddef.h
        Source/LibJXR/common/include/wmspecstring.h
        Source/LibJXR/common/include/wmspecstrings_adt.h
        Source/LibJXR/common/include/wmspecstrings_strict.h
        Source/LibJXR/common/include/wmspecstrings_undef.h
        Source/LibJXR/image/decode/decode.h
        Source/LibJXR/image/encode/encode.h
        Source/LibJXR/image/sys/ansi.h
        Source/LibJXR/image/sys/common.h
        Source/LibJXR/image/sys/perfTimer.h
        Source/LibJXR/image/sys/strcodec.h
        Source/LibJXR/image/sys/strTransform.h
        Source/LibJXR/image/sys/windowsmediaphoto.h
        Source/LibJXR/image/sys/xplatform_image.h
        Source/LibJXR/image/x86/x86.h
        Source/LibJXR/jxrgluelib/JXRGlue.h
        Source/LibJXR/jxrgluelib/JXRMeta.h
    )
    list(APPEND FreeImage_SOURCES ${FreeImage_JXR_SRCS})
    include_directories(Source/LibJXR)
    include_directories(Source/LibJXR/image/sys)
    include_directories(Source/LibJXR/common/include)
    include_directories(Source/LibJXR/jxrgluelib)
    add_definitions(-DINCLUDE_LIB_JXR)
    endif()
endif()

if(BUILD_WEBP)
    if(USE_SYSTEM_WEBP)
        find_package(WebP CONFIG QUIET)
        if(NOT WebP_FOUND)
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(WEBP REQUIRED IMPORTED_TARGET libwebp libwebpdemux libwebpmux)
            list(APPEND FreeImage_LIBS PkgConfig::WEBP)
        else()
            # The mux target's name has varied across libwebp CMake config
            # versions (WebP::webpmux vs. WebP::libwebpmux) - probe for both.
            list(APPEND FreeImage_LIBS WebP::webp WebP::webpdemux)
            if(TARGET WebP::webpmux)
                list(APPEND FreeImage_LIBS WebP::webpmux)
            elseif(TARGET WebP::libwebpmux)
                list(APPEND FreeImage_LIBS WebP::libwebpmux)
            endif()
        endif()
        add_definitions(-DFREEIMAGE_SYSTEM_WEBP -DINCLUDE_LIB_WEBP)
    else()
    set(FreeImage_WEBP_SRCS
        ./Source/LibWebP/sharpyuv/sharpyuv_cpu.c ./Source/LibWebP/sharpyuv/sharpyuv_cpu.h
        ./Source/LibWebP/sharpyuv/sharpyuv_csp.c ./Source/LibWebP/sharpyuv/sharpyuv_csp.h
        ./Source/LibWebP/sharpyuv/sharpyuv_dsp.c ./Source/LibWebP/sharpyuv/sharpyuv_dsp.h
        ./Source/LibWebP/sharpyuv/sharpyuv_gamma.c ./Source/LibWebP/sharpyuv/sharpyuv_gamma.h
        ./Source/LibWebP/sharpyuv/sharpyuv_neon.c ./Source/LibWebP/sharpyuv/sharpyuv_sse2.c
        ./Source/LibWebP/sharpyuv/sharpyuv.c ./Source/LibWebP/sharpyuv/sharpyuv.h
        ./Source/LibWebP/src/dec/alpha_dec.c ./Source/LibWebP/src/dec/alphai_dec.h
        ./Source/LibWebP/src/dec/buffer_dec.c ./Source/LibWebP/src/dec/common_dec.h
        ./Source/LibWebP/src/dec/frame_dec.c ./Source/LibWebP/src/dec/idec_dec.c
        ./Source/LibWebP/src/dec/io_dec.c ./Source/LibWebP/src/dec/quant_dec.c
        ./Source/LibWebP/src/dec/tree_dec.c ./Source/LibWebP/src/dec/vp8_dec.c
        ./Source/LibWebP/src/dec/vp8_dec.h ./Source/LibWebP/src/dec/vp8i_dec.h
        ./Source/LibWebP/src/dec/vp8l_dec.c ./Source/LibWebP/src/dec/vp8li_dec.h
        ./Source/LibWebP/src/dec/webp_dec.c ./Source/LibWebP/src/dec/webpi_dec.h
        ./Source/LibWebP/src/demux/anim_decode.c ./Source/LibWebP/src/demux/demux.c
        ./Source/LibWebP/src/dsp/alpha_processing_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/alpha_processing_neon.c
        ./Source/LibWebP/src/dsp/alpha_processing_sse2.c
        ./Source/LibWebP/src/dsp/alpha_processing_sse41.c
        ./Source/LibWebP/src/dsp/alpha_processing.c ./Source/LibWebP/src/dsp/common_sse2.h
        ./Source/LibWebP/src/dsp/common_sse41.h ./Source/LibWebP/src/dsp/cost_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/cost_mips32.c ./Source/LibWebP/src/dsp/cost_neon.c
        ./Source/LibWebP/src/dsp/cost_sse2.c ./Source/LibWebP/src/dsp/cost.c
        ./Source/LibWebP/src/dsp/cpu.c ./Source/LibWebP/src/dsp/cpu.h
        ./Source/LibWebP/src/dsp/dec_clip_tables.c ./Source/LibWebP/src/dsp/dec_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/dec_mips32.c ./Source/LibWebP/src/dsp/dec_msa.c
        ./Source/LibWebP/src/dsp/dec_neon.c ./Source/LibWebP/src/dsp/dec_sse2.c
        ./Source/LibWebP/src/dsp/dec_sse41.c ./Source/LibWebP/src/dsp/dec.c
        ./Source/LibWebP/src/dsp/dsp.h ./Source/LibWebP/src/dsp/enc_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/enc_mips32.c ./Source/LibWebP/src/dsp/enc_msa.c
        ./Source/LibWebP/src/dsp/enc_neon.c ./Source/LibWebP/src/dsp/enc_sse2.c
        ./Source/LibWebP/src/dsp/enc_sse41.c ./Source/LibWebP/src/dsp/enc.c
        ./Source/LibWebP/src/dsp/filters_mips_dsp_r2.c ./Source/LibWebP/src/dsp/filters_msa.c
        ./Source/LibWebP/src/dsp/filters_neon.c ./Source/LibWebP/src/dsp/filters_sse2.c
        ./Source/LibWebP/src/dsp/filters.c ./Source/LibWebP/src/dsp/lossless_avx2.c
        ./Source/LibWebP/src/dsp/lossless_common.h ./Source/LibWebP/src/dsp/lossless_enc_avx2.c
        ./Source/LibWebP/src/dsp/lossless_enc_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/lossless_enc_mips32.c
        ./Source/LibWebP/src/dsp/lossless_enc_msa.c
        ./Source/LibWebP/src/dsp/lossless_enc_neon.c
        ./Source/LibWebP/src/dsp/lossless_enc_sse2.c
        ./Source/LibWebP/src/dsp/lossless_enc_sse41.c ./Source/LibWebP/src/dsp/lossless_enc.c
        ./Source/LibWebP/src/dsp/lossless_mips_dsp_r2.c ./Source/LibWebP/src/dsp/lossless_msa.c
        ./Source/LibWebP/src/dsp/lossless_neon.c ./Source/LibWebP/src/dsp/lossless_sse2.c
        ./Source/LibWebP/src/dsp/lossless_sse41.c ./Source/LibWebP/src/dsp/lossless.c
        ./Source/LibWebP/src/dsp/lossless.h ./Source/LibWebP/src/dsp/mips_macro.h
        ./Source/LibWebP/src/dsp/msa_macro.h ./Source/LibWebP/src/dsp/neon.h
        ./Source/LibWebP/src/dsp/quant.h ./Source/LibWebP/src/dsp/rescaler_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/rescaler_mips32.c ./Source/LibWebP/src/dsp/rescaler_msa.c
        ./Source/LibWebP/src/dsp/rescaler_neon.c ./Source/LibWebP/src/dsp/rescaler_sse2.c
        ./Source/LibWebP/src/dsp/rescaler.c ./Source/LibWebP/src/dsp/ssim_sse2.c
        ./Source/LibWebP/src/dsp/ssim.c ./Source/LibWebP/src/dsp/upsampling_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/upsampling_msa.c ./Source/LibWebP/src/dsp/upsampling_neon.c
        ./Source/LibWebP/src/dsp/upsampling_sse2.c ./Source/LibWebP/src/dsp/upsampling_sse41.c
        ./Source/LibWebP/src/dsp/upsampling.c ./Source/LibWebP/src/dsp/yuv_mips_dsp_r2.c
        ./Source/LibWebP/src/dsp/yuv_mips32.c ./Source/LibWebP/src/dsp/yuv_neon.c
        ./Source/LibWebP/src/dsp/yuv_sse2.c ./Source/LibWebP/src/dsp/yuv_sse41.c
        ./Source/LibWebP/src/dsp/yuv.c ./Source/LibWebP/src/dsp/yuv.h
        ./Source/LibWebP/src/enc/alpha_enc.c ./Source/LibWebP/src/enc/analysis_enc.c
        ./Source/LibWebP/src/enc/backward_references_cost_enc.c
        ./Source/LibWebP/src/enc/backward_references_enc.c
        ./Source/LibWebP/src/enc/backward_references_enc.h
        ./Source/LibWebP/src/enc/config_enc.c ./Source/LibWebP/src/enc/cost_enc.c
        ./Source/LibWebP/src/enc/cost_enc.h ./Source/LibWebP/src/enc/filter_enc.c
        ./Source/LibWebP/src/enc/frame_enc.c ./Source/LibWebP/src/enc/histogram_enc.c
        ./Source/LibWebP/src/enc/histogram_enc.h ./Source/LibWebP/src/enc/iterator_enc.c
        ./Source/LibWebP/src/enc/near_lossless_enc.c ./Source/LibWebP/src/enc/picture_csp_enc.c
        ./Source/LibWebP/src/enc/picture_enc.c ./Source/LibWebP/src/enc/picture_psnr_enc.c
        ./Source/LibWebP/src/enc/picture_rescale_enc.c
        ./Source/LibWebP/src/enc/picture_tools_enc.c ./Source/LibWebP/src/enc/predictor_enc.c
        ./Source/LibWebP/src/enc/quant_enc.c ./Source/LibWebP/src/enc/syntax_enc.c
        ./Source/LibWebP/src/enc/token_enc.c ./Source/LibWebP/src/enc/tree_enc.c
        ./Source/LibWebP/src/enc/vp8i_enc.h ./Source/LibWebP/src/enc/vp8l_enc.c
        ./Source/LibWebP/src/enc/vp8li_enc.h ./Source/LibWebP/src/enc/webp_enc.c
        ./Source/LibWebP/src/mux/anim_encode.c ./Source/LibWebP/src/mux/animi.h
        ./Source/LibWebP/src/mux/muxedit.c ./Source/LibWebP/src/mux/muxi.h
        ./Source/LibWebP/src/mux/muxinternal.c ./Source/LibWebP/src/mux/muxread.c
        ./Source/LibWebP/src/utils/bit_reader_inl_utils.h
        ./Source/LibWebP/src/utils/bit_reader_utils.c
        ./Source/LibWebP/src/utils/bit_reader_utils.h
        ./Source/LibWebP/src/utils/bit_writer_utils.c
        ./Source/LibWebP/src/utils/bit_writer_utils.h
        ./Source/LibWebP/src/utils/color_cache_utils.c
        ./Source/LibWebP/src/utils/color_cache_utils.h
        ./Source/LibWebP/src/utils/endian_inl_utils.h
        ./Source/LibWebP/src/utils/filters_utils.c ./Source/LibWebP/src/utils/filters_utils.h
        ./Source/LibWebP/src/utils/huffman_encode_utils.c
        ./Source/LibWebP/src/utils/huffman_encode_utils.h
        ./Source/LibWebP/src/utils/huffman_utils.c ./Source/LibWebP/src/utils/huffman_utils.h
        ./Source/LibWebP/src/utils/palette.c ./Source/LibWebP/src/utils/palette.h
        ./Source/LibWebP/src/utils/quant_levels_dec_utils.c
        ./Source/LibWebP/src/utils/quant_levels_dec_utils.h
        ./Source/LibWebP/src/utils/quant_levels_utils.c
        ./Source/LibWebP/src/utils/quant_levels_utils.h
        ./Source/LibWebP/src/utils/random_utils.c ./Source/LibWebP/src/utils/random_utils.h
        ./Source/LibWebP/src/utils/rescaler_utils.c ./Source/LibWebP/src/utils/rescaler_utils.h
        ./Source/LibWebP/src/utils/thread_utils.c ./Source/LibWebP/src/utils/thread_utils.h
        ./Source/LibWebP/src/utils/utils.c ./Source/LibWebP/src/utils/utils.h
        ./Source/LibWebP/src/webp/decode.h ./Source/LibWebP/src/webp/demux.h
        ./Source/LibWebP/src/webp/encode.h ./Source/LibWebP/src/webp/format_constants.h
        ./Source/LibWebP/src/webp/mux_types.h ./Source/LibWebP/src/webp/mux.h
        ./Source/LibWebP/src/webp/types.h
    )
    list(APPEND FreeImage_SOURCES ${FreeImage_WEBP_SRCS})
    include_directories(Source/LibWebP)
    # LibWebP's own sources include each other as "src/dec/foo.h" (relative
    # to Source/LibWebP above), but PluginWebP.cpp includes the public API
    # as <webp/decode.h>, which needs Source/LibWebP/src on the path too.
    include_directories(Source/LibWebP/src)
    add_definitions(-DINCLUDE_LIB_WEBP)

    # Real MSVC doesn't gate SIMD intrinsics on /arch:, but clang-cl (which
    # also defines _MSC_VER) does enforce per-function target features - so
    # LibWebP's hand-dispatched SSE4.1/AVX2 sources need the matching -m
    # flags explicitly under clang-cl, passed through via its /clang: prefix.
    if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        foreach(_webp_simd_src ${FreeImage_WEBP_SRCS})
            if(_webp_simd_src MATCHES "_sse41\\.c$")
                set_source_files_properties(${_webp_simd_src} PROPERTIES COMPILE_OPTIONS "/clang:-msse4.1")
            elseif(_webp_simd_src MATCHES "_avx2\\.c$")
                set_source_files_properties(${_webp_simd_src} PROPERTIES COMPILE_OPTIONS "/clang:-mavx2")
            endif()
        endforeach()
    endif()
    endif()
endif()

if(BUILD_LIBRAWLITE)
    if(USE_SYSTEM_LIBRAWLITE)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBRAW REQUIRED IMPORTED_TARGET libraw)
        list(APPEND FreeImage_LIBS PkgConfig::LIBRAW)
        add_definitions(-DFREEIMAGE_SYSTEM_LIBRAWLITE -DINCLUDE_LIB_RAW)
    else()
    set(FreeImage_LIBRAWLITE_SRCS
        ./Source/LibRawLite/internal/dcraw_defs.h
        ./Source/LibRawLite/internal/dcraw_fileio_defs.h ./Source/LibRawLite/internal/defines.h
        ./Source/LibRawLite/internal/dmp_include.h
        ./Source/LibRawLite/internal/libraw_cameraids.h
        ./Source/LibRawLite/internal/libraw_checked_buffer.h
        ./Source/LibRawLite/internal/libraw_cxx_defs.h
        ./Source/LibRawLite/internal/libraw_internal_funcs.h
        ./Source/LibRawLite/internal/losslessjpeg.h ./Source/LibRawLite/internal/var_defines.h
        ./Source/LibRawLite/internal/x3f_tools.h ./Source/LibRawLite/libraw/libraw.h
        ./Source/LibRawLite/libraw/libraw_alloc.h ./Source/LibRawLite/libraw/libraw_const.h
        ./Source/LibRawLite/libraw/libraw_datastream.h
        ./Source/LibRawLite/libraw/libraw_internal.h ./Source/LibRawLite/libraw/libraw_types.h
        ./Source/LibRawLite/libraw/libraw_version.h
        ./Source/LibRawLite/src/decoders/canon_600.cpp ./Source/LibRawLite/src/decoders/crx.cpp
        ./Source/LibRawLite/src/decoders/decoders_dcraw.cpp
        ./Source/LibRawLite/src/decoders/decoders_libraw.cpp
        ./Source/LibRawLite/src/decoders/decoders_libraw_dcrdefs.cpp
        ./Source/LibRawLite/src/decoders/dng.cpp ./Source/LibRawLite/src/decoders/fp_dng.cpp
        ./Source/LibRawLite/src/decoders/fuji_compressed.cpp
        ./Source/LibRawLite/src/decoders/generic.cpp
        ./Source/LibRawLite/src/decoders/kodak_decoders.cpp
        ./Source/LibRawLite/src/decoders/load_mfbacks.cpp
        ./Source/LibRawLite/src/decoders/olympus14.cpp
        ./Source/LibRawLite/src/decoders/pana8.cpp ./Source/LibRawLite/src/decoders/smal.cpp
        ./Source/LibRawLite/src/decoders/sonycc.cpp ./Source/LibRawLite/src/decoders/unpack.cpp
        ./Source/LibRawLite/src/decoders/unpack_thumb.cpp
        ./Source/LibRawLite/src/decompressors/losslessjpeg.cpp
        ./Source/LibRawLite/src/demosaic/aahd_demosaic.cpp
        ./Source/LibRawLite/src/demosaic/ahd_demosaic.cpp
        ./Source/LibRawLite/src/demosaic/dcb_demosaic.cpp
        ./Source/LibRawLite/src/demosaic/dht_demosaic.cpp
        ./Source/LibRawLite/src/demosaic/misc_demosaic.cpp
        ./Source/LibRawLite/src/demosaic/xtrans_demosaic.cpp
        ./Source/LibRawLite/src/integration/dngsdk_glue.cpp
        ./Source/LibRawLite/src/integration/rawspeed_glue.cpp
        ./Source/LibRawLite/src/libraw_c_api.cpp ./Source/LibRawLite/src/libraw_datastream.cpp
        ./Source/LibRawLite/src/metadata/adobepano.cpp
        ./Source/LibRawLite/src/metadata/canon.cpp ./Source/LibRawLite/src/metadata/ciff.cpp
        ./Source/LibRawLite/src/metadata/cr3_parser.cpp
        ./Source/LibRawLite/src/metadata/epson.cpp
        ./Source/LibRawLite/src/metadata/exif_gps.cpp ./Source/LibRawLite/src/metadata/fuji.cpp
        ./Source/LibRawLite/src/metadata/hasselblad_model.cpp
        ./Source/LibRawLite/src/metadata/identify.cpp
        ./Source/LibRawLite/src/metadata/identify_tools.cpp
        ./Source/LibRawLite/src/metadata/kodak.cpp ./Source/LibRawLite/src/metadata/leica.cpp
        ./Source/LibRawLite/src/metadata/makernotes.cpp
        ./Source/LibRawLite/src/metadata/mediumformat.cpp
        ./Source/LibRawLite/src/metadata/minolta.cpp
        ./Source/LibRawLite/src/metadata/misc_parsers.cpp
        ./Source/LibRawLite/src/metadata/nikon.cpp
        ./Source/LibRawLite/src/metadata/normalize_model.cpp
        ./Source/LibRawLite/src/metadata/olympus.cpp ./Source/LibRawLite/src/metadata/p1.cpp
        ./Source/LibRawLite/src/metadata/pentax.cpp
        ./Source/LibRawLite/src/metadata/samsung.cpp ./Source/LibRawLite/src/metadata/sony.cpp
        ./Source/LibRawLite/src/metadata/tiff.cpp
        ./Source/LibRawLite/src/postprocessing/aspect_ratio.cpp
        ./Source/LibRawLite/src/postprocessing/dcraw_process.cpp
        ./Source/LibRawLite/src/postprocessing/mem_image.cpp
        ./Source/LibRawLite/src/postprocessing/postprocessing_aux.cpp
        ./Source/LibRawLite/src/postprocessing/postprocessing_utils.cpp
        ./Source/LibRawLite/src/postprocessing/postprocessing_utils_dcrdefs.cpp
        ./Source/LibRawLite/src/preprocessing/ext_preprocess.cpp
        ./Source/LibRawLite/src/preprocessing/raw2image.cpp
        ./Source/LibRawLite/src/preprocessing/subtract_black.cpp
        ./Source/LibRawLite/src/tables/cameralist.cpp
        ./Source/LibRawLite/src/tables/colorconst.cpp
        ./Source/LibRawLite/src/tables/colordata.cpp ./Source/LibRawLite/src/tables/wblists.cpp
        ./Source/LibRawLite/src/utils/curves.cpp ./Source/LibRawLite/src/utils/decoder_info.cpp
        ./Source/LibRawLite/src/utils/init_close_utils.cpp
        ./Source/LibRawLite/src/utils/open.cpp
        ./Source/LibRawLite/src/utils/phaseone_processing.cpp
        ./Source/LibRawLite/src/utils/read_utils.cpp
        ./Source/LibRawLite/src/utils/thumb_utils.cpp
        ./Source/LibRawLite/src/utils/utils_dcraw.cpp
        ./Source/LibRawLite/src/utils/utils_libraw.cpp
        ./Source/LibRawLite/src/write/apply_profile.cpp
        ./Source/LibRawLite/src/write/file_write.cpp
        ./Source/LibRawLite/src/write/tiff_writer.cpp
        ./Source/LibRawLite/src/x3f/x3f_parse_process.cpp
        ./Source/LibRawLite/src/x3f/x3f_utils_patched.cpp
    )
    list(APPEND FreeImage_SOURCES ${FreeImage_LIBRAWLITE_SRCS})
    include_directories(Source/LibRawLite)
    include_directories(Source/LibRawLite/dcraw)
    include_directories(Source/LibRawLite/internal)
    include_directories(Source/LibRawLite/libraw)
    include_directories(Source/LibRawLite/src)
    # Bundled into FreeImage, not built as its own DLL. On MSVC and
    # clang-cl, libraw.h otherwise marks the definitions dllimport
    # (C2491). Makefile.mingw passes the same flag.
    add_definitions(-DINCLUDE_LIB_RAW -DLIBRAW_NODLL)

    add_definitions(-DUSE_X3FTOOLS)
    endif()
endif()

    set(FreeImage_OPENEXR_HALF_SRCS
        ./Source/OpenEXR/Half/ImathConfig.h ./Source/OpenEXR/Half/ImathExport.h 
        ./Source/OpenEXR/Half/ImathNamespace.h ./Source/OpenEXR/Half/ImathPlatform.h 
        ./Source/OpenEXR/Half/half.cpp ./Source/OpenEXR/Half/half.h 
        ./Source/OpenEXR/Half/halfFunction.h ./Source/OpenEXR/Half/halfLimits.h 
        ./Source/OpenEXR/Half/toFloat.h 
    )
list(APPEND FreeImage_SOURCES ${FreeImage_OPENEXR_HALF_SRCS})
# Not added to the global include path: half.cpp includes its own headers
# with quotes (resolved relative to its own directory), and PluginTIFF.cpp/
# PluginEXR.cpp's bundled-mode "../OpenEXR/Half/half.h" include is likewise
# a quoted relative path. Keeping this off the global -I search path avoids
# it winning over system Imath's <half.h> when USE_SYSTEM_OPENEXR pulls in
# an unqualified <half.h> transitively.

if(BUILD_OPENEXR)
    if(USE_SYSTEM_OPENEXR)
        find_package(Imath CONFIG REQUIRED)
        find_package(OpenEXR CONFIG REQUIRED)
        list(APPEND FreeImage_LIBS OpenEXR::OpenEXR Imath::Imath)
        add_definitions(-DFREEIMAGE_SYSTEM_OPENEXR -DINCLUDE_LIB_OPENEXR)
    else()
        # The bundled OpenEXR 3.5.1's IlmImf (ImfMultiPartInputFile.cpp) uses
        # std::any, which requires C++17 - this is upstream's own minimum,
        # not something this project imposes. BUILD_OPENEXR defaults OFF, so
        # this only matters if a caller explicitly opts in. Error out if an
        # explicit older standard was requested; otherwise default to C++17
        # so BUILD_OPENEXR=ON works out of the box without also requiring
        # -DCPP_STANDARD=17.
        if(CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD LESS 17)
            message(FATAL_ERROR "BUILD_OPENEXR requires C++17 or newer (OpenEXR's IlmImf uses std::any) - CMAKE_CXX_STANDARD is currently ${CMAKE_CXX_STANDARD}.")
        elseif(NOT CMAKE_CXX_STANDARD)
            set(CMAKE_CXX_STANDARD 17)
        endif()

    set(FreeImage_IMATH_SRCS
        ./Source/Imath/ImathBox.h ./Source/Imath/ImathBoxAlgo.h ./Source/Imath/ImathColor.h 
        ./Source/Imath/ImathColorAlgo.cpp ./Source/Imath/ImathColorAlgo.h 
        ./Source/Imath/ImathConfig.h ./Source/Imath/ImathEuler.h ./Source/Imath/ImathExport.h 
        ./Source/Imath/ImathForward.h ./Source/Imath/ImathFrame.h ./Source/Imath/ImathFrustum.h 
        ./Source/Imath/ImathFrustumTest.h ./Source/Imath/ImathFun.cpp ./Source/Imath/ImathFun.h 
        ./Source/Imath/ImathGL.h ./Source/Imath/ImathGLU.h ./Source/Imath/ImathInt64.h 
        ./Source/Imath/ImathInterval.h ./Source/Imath/ImathLine.h 
        ./Source/Imath/ImathLineAlgo.h ./Source/Imath/ImathMath.h ./Source/Imath/ImathMatrix.h 
        ./Source/Imath/ImathMatrixAlgo.cpp ./Source/Imath/ImathMatrixAlgo.h 
        ./Source/Imath/ImathNamespace.h ./Source/Imath/ImathPlane.h 
        ./Source/Imath/ImathPlatform.h ./Source/Imath/ImathQuat.h 
        ./Source/Imath/ImathRandom.cpp ./Source/Imath/ImathRandom.h ./Source/Imath/ImathRoots.h 
        ./Source/Imath/ImathShear.h ./Source/Imath/ImathSphere.h 
        ./Source/Imath/ImathTypeTraits.h ./Source/Imath/ImathVec.h 
        ./Source/Imath/ImathVecAlgo.h 
    )
    set(FreeImage_IEX_SRCS
        ./Source/OpenEXR/Iex/Iex.h
        ./Source/OpenEXR/Iex/IexBaseExc.cpp
        ./Source/OpenEXR/Iex/IexBaseExc.h
        ./Source/OpenEXR/Iex/IexConfigInternal.h
        ./Source/OpenEXR/Iex/IexErrnoExc.h
        ./Source/OpenEXR/Iex/IexExport.h
        ./Source/OpenEXR/Iex/IexForward.h
        ./Source/OpenEXR/Iex/IexMacros.h
        ./Source/OpenEXR/Iex/IexMathExc.h
        ./Source/OpenEXR/Iex/IexMathFloatExc.cpp
        ./Source/OpenEXR/Iex/IexMathFloatExc.h
        ./Source/OpenEXR/Iex/IexMathFpu.cpp
        ./Source/OpenEXR/Iex/IexMathFpu.h
        ./Source/OpenEXR/Iex/IexMathIeeeExc.h
        ./Source/OpenEXR/Iex/IexNamespace.h
        ./Source/OpenEXR/Iex/IexThrowErrnoExc.cpp
        ./Source/OpenEXR/Iex/IexThrowErrnoExc.h
    )
    set(FreeImage_ILMTHREAD_SRCS
        ./Source/OpenEXR/IlmThread/IlmThread.cpp
        ./Source/OpenEXR/IlmThread/IlmThread.h
        ./Source/OpenEXR/IlmThread/IlmThreadExport.h
        ./Source/OpenEXR/IlmThread/IlmThreadForward.h
        ./Source/OpenEXR/IlmThread/IlmThreadMutex.h
        ./Source/OpenEXR/IlmThread/IlmThreadNamespace.h
        ./Source/OpenEXR/IlmThread/IlmThreadPool.cpp
        ./Source/OpenEXR/IlmThread/IlmThreadPool.h
        ./Source/OpenEXR/IlmThread/IlmThreadProcessGroup.h
        ./Source/OpenEXR/IlmThread/IlmThreadSemaphore.cpp
        ./Source/OpenEXR/IlmThread/IlmThreadSemaphore.h
        ./Source/OpenEXR/IlmThread/IlmThreadSemaphoreOSX.cpp
        ./Source/OpenEXR/IlmThread/IlmThreadSemaphorePosix.cpp
        ./Source/OpenEXR/IlmThread/IlmThreadSemaphorePosixCompat.cpp
        ./Source/OpenEXR/IlmThread/IlmThreadSemaphoreWin32.cpp
    )
    set(FreeImage_OPENEXRCORE_SRCS
        ./Source/OpenEXR/OpenEXRCore/attributes.c
        ./Source/OpenEXR/OpenEXRCore/backward_compatibility.h
        ./Source/OpenEXR/OpenEXRCore/base.c
        ./Source/OpenEXR/OpenEXRCore/bytes.c
        ./Source/OpenEXR/OpenEXRCore/channel_list.c
        ./Source/OpenEXR/OpenEXRCore/chunk.c
        ./Source/OpenEXR/OpenEXRCore/coding.c
        ./Source/OpenEXR/OpenEXRCore/color_metadata.c
        ./Source/OpenEXR/OpenEXRCore/compression.c
        ./Source/OpenEXR/OpenEXRCore/context.c
        ./Source/OpenEXR/OpenEXRCore/debug.c
        ./Source/OpenEXR/OpenEXRCore/decoding.c
        ./Source/OpenEXR/OpenEXRCore/encoding.c
        ./Source/OpenEXR/OpenEXRCore/float_vector.c
        ./Source/OpenEXR/OpenEXRCore/internal_attr.h
        ./Source/OpenEXR/OpenEXRCore/internal_b44.c
        ./Source/OpenEXR/OpenEXRCore/internal_b44_table.c
        ./Source/OpenEXR/OpenEXRCore/internal_b44_table_init.c
        ./Source/OpenEXR/OpenEXRCore/internal_bytes.h
        ./Source/OpenEXR/OpenEXRCore/internal_channel_list.h
        ./Source/OpenEXR/OpenEXRCore/internal_coding.h
        ./Source/OpenEXR/OpenEXRCore/internal_compress.h
        ./Source/OpenEXR/OpenEXRCore/internal_constants.h
        ./Source/OpenEXR/OpenEXRCore/internal_cpuid.h
        ./Source/OpenEXR/OpenEXRCore/internal_decompress.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa.c
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_channeldata.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_classifier.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_compressor.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_decoder.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_encoder.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_helpers.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_simd.h
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_table.c
        ./Source/OpenEXR/OpenEXRCore/internal_dwa_table_init.c
        ./Source/OpenEXR/OpenEXRCore/internal_file.h
        ./Source/OpenEXR/OpenEXRCore/internal_float_vector.h
        ./Source/OpenEXR/OpenEXRCore/internal_ht.cpp
        ./Source/OpenEXR/OpenEXRCore/internal_ht_common.cpp
        ./Source/OpenEXR/OpenEXRCore/internal_huf.c
        ./Source/OpenEXR/OpenEXRCore/internal_huf.h
        ./Source/OpenEXR/OpenEXRCore/internal_memory.h
        ./Source/OpenEXR/OpenEXRCore/internal_opaque.h
        ./Source/OpenEXR/OpenEXRCore/internal_piz.c
        ./Source/OpenEXR/OpenEXRCore/internal_posix_file_impl.h
        ./Source/OpenEXR/OpenEXRCore/internal_preview.h
        ./Source/OpenEXR/OpenEXRCore/internal_pxr24.c
        ./Source/OpenEXR/OpenEXRCore/internal_rle.c
        ./Source/OpenEXR/OpenEXRCore/internal_string.h
        ./Source/OpenEXR/OpenEXRCore/internal_string_vector.h
        ./Source/OpenEXR/OpenEXRCore/internal_structs.c
        ./Source/OpenEXR/OpenEXRCore/internal_structs.h
        ./Source/OpenEXR/OpenEXRCore/internal_thread.h
        ./Source/OpenEXR/OpenEXRCore/internal_util.h
        ./Source/OpenEXR/OpenEXRCore/internal_win32_file_impl.h
        ./Source/OpenEXR/OpenEXRCore/internal_xdr.h
        ./Source/OpenEXR/OpenEXRCore/internal_zip.c
        ./Source/OpenEXR/OpenEXRCore/internal_zstd.c
        ./Source/OpenEXR/OpenEXRCore/internal_zstd_delta.c
        ./Source/OpenEXR/OpenEXRCore/internal_zstd_shuffle.c
        ./Source/OpenEXR/OpenEXRCore/memory.c
        ./Source/OpenEXR/OpenEXRCore/opaque.c
        ./Source/OpenEXR/OpenEXRCore/openexr.h
        ./Source/OpenEXR/OpenEXRCore/openexr_attr.h
        ./Source/OpenEXR/OpenEXRCore/openexr_base.h
        ./Source/OpenEXR/OpenEXRCore/openexr_chunkio.h
        ./Source/OpenEXR/OpenEXRCore/openexr_coding.h
        ./Source/OpenEXR/OpenEXRCore/openexr_color_metadata.h
        ./Source/OpenEXR/OpenEXRCore/openexr_compression.h
        ./Source/OpenEXR/OpenEXRCore/openexr_config.h
        ./Source/OpenEXR/OpenEXRCore/openexr_context.h
        ./Source/OpenEXR/OpenEXRCore/openexr_debug.h
        ./Source/OpenEXR/OpenEXRCore/openexr_decode.h
        ./Source/OpenEXR/OpenEXRCore/openexr_encode.h
        ./Source/OpenEXR/OpenEXRCore/openexr_errors.h
        ./Source/OpenEXR/OpenEXRCore/openexr_part.h
        ./Source/OpenEXR/OpenEXRCore/openexr_std_attr.h
        ./Source/OpenEXR/OpenEXRCore/openexr_version.h
        ./Source/OpenEXR/OpenEXRCore/pack.c
        ./Source/OpenEXR/OpenEXRCore/parse_header.c
        ./Source/OpenEXR/OpenEXRCore/part.c
        ./Source/OpenEXR/OpenEXRCore/part_attr.c
        ./Source/OpenEXR/OpenEXRCore/preview.c
        ./Source/OpenEXR/OpenEXRCore/std_attr.c
        ./Source/OpenEXR/OpenEXRCore/string.c
        ./Source/OpenEXR/OpenEXRCore/string_vector.c
        ./Source/OpenEXR/OpenEXRCore/unpack.c
        ./Source/OpenEXR/OpenEXRCore/validation.c
        ./Source/OpenEXR/OpenEXRCore/write_header.c
    )
    set(FreeImage_LIBDEFLATE_SRCS
        ./Source/LibDeflate/common_defs.h
        ./Source/LibDeflate/lib/adler32.c
        ./Source/LibDeflate/lib/arm/adler32_impl.h
        ./Source/LibDeflate/lib/arm/cpu_features.c
        ./Source/LibDeflate/lib/arm/cpu_features.h
        ./Source/LibDeflate/lib/arm/crc32_impl.h
        ./Source/LibDeflate/lib/arm/crc32_pmull_helpers.h
        ./Source/LibDeflate/lib/arm/crc32_pmull_wide.h
        ./Source/LibDeflate/lib/arm/matchfinder_impl.h
        ./Source/LibDeflate/lib/bt_matchfinder.h
        ./Source/LibDeflate/lib/cpu_features_common.h
        ./Source/LibDeflate/lib/crc32.c
        ./Source/LibDeflate/lib/crc32_multipliers.h
        ./Source/LibDeflate/lib/crc32_tables.h
        ./Source/LibDeflate/lib/decompress_template.h
        ./Source/LibDeflate/lib/deflate_compress.c
        ./Source/LibDeflate/lib/deflate_compress.h
        ./Source/LibDeflate/lib/deflate_constants.h
        ./Source/LibDeflate/lib/deflate_decompress.c
        ./Source/LibDeflate/lib/gzip_compress.c
        ./Source/LibDeflate/lib/gzip_constants.h
        ./Source/LibDeflate/lib/gzip_decompress.c
        ./Source/LibDeflate/lib/hc_matchfinder.h
        ./Source/LibDeflate/lib/ht_matchfinder.h
        ./Source/LibDeflate/lib/lib_common.h
        ./Source/LibDeflate/lib/matchfinder_common.h
        ./Source/LibDeflate/lib/riscv/matchfinder_impl.h
        ./Source/LibDeflate/lib/utils.c
        ./Source/LibDeflate/lib/x86/adler32_impl.h
        ./Source/LibDeflate/lib/x86/adler32_template.h
        ./Source/LibDeflate/lib/x86/cpu_features.c
        ./Source/LibDeflate/lib/x86/cpu_features.h
        ./Source/LibDeflate/lib/x86/crc32_impl.h
        ./Source/LibDeflate/lib/x86/crc32_pclmul_template.h
        ./Source/LibDeflate/lib/x86/decompress_impl.h
        ./Source/LibDeflate/lib/x86/matchfinder_impl.h
        ./Source/LibDeflate/lib/zlib_compress.c
        ./Source/LibDeflate/lib/zlib_constants.h
        ./Source/LibDeflate/lib/zlib_decompress.c
        ./Source/LibDeflate/libdeflate.h
    )
    # libdeflate 1.26 requires C11 even when the consumer requests C99 for
    # FreeImage's other C sources. Source options follow the target's flags.
    set_property(SOURCE ${FreeImage_LIBDEFLATE_SRCS} APPEND PROPERTY COMPILE_OPTIONS
        "$<$<COMPILE_LANG_AND_ID:C,MSVC>:/std:c11>"
        "$<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-std=c11>")
    # Auxiliary codecs vendored by OpenEXR 3.5.1. OpenJPH SIMD kernels are
    # selected separately, with portable implementations for other targets.
    set(FreeImage_ZSTD_SRCS
        ./Source/OpenEXR/Zstd/lib/common/debug.c
        ./Source/OpenEXR/Zstd/lib/common/entropy_common.c
        ./Source/OpenEXR/Zstd/lib/common/error_private.c
        ./Source/OpenEXR/Zstd/lib/common/fse_decompress.c
        ./Source/OpenEXR/Zstd/lib/common/pool.c
        ./Source/OpenEXR/Zstd/lib/common/threading.c
        ./Source/OpenEXR/Zstd/lib/common/xxhash.c
        ./Source/OpenEXR/Zstd/lib/common/zstd_common.c
        ./Source/OpenEXR/Zstd/lib/compress/fse_compress.c
        ./Source/OpenEXR/Zstd/lib/compress/hist.c
        ./Source/OpenEXR/Zstd/lib/compress/huf_compress.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_compress.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_compress_literals.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_compress_sequences.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_compress_superblock.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_double_fast.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_fast.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_lazy.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_ldm.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_opt.c
        ./Source/OpenEXR/Zstd/lib/compress/zstd_preSplit.c
        ./Source/OpenEXR/Zstd/lib/compress/zstdmt_compress.c
        ./Source/OpenEXR/Zstd/lib/decompress/huf_decompress.c
        ./Source/OpenEXR/Zstd/lib/decompress/zstd_ddict.c
        ./Source/OpenEXR/Zstd/lib/decompress/zstd_decompress.c
        ./Source/OpenEXR/Zstd/lib/decompress/zstd_decompress_block.c
    )
    set(FreeImage_OPENJPH_SRCS
        ./Source/OpenEXR/OpenJPH/codestream/ojph_bitbuffer_read.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_bitbuffer_write.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codeblock.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codeblock.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codeblock_fun.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codeblock_fun.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codestream.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codestream_gen.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codestream_local.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_codestream_local.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_params.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_params_local.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_precinct.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_precinct.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_resolution.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_resolution.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_subband.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_subband.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_tile.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_tile.h
        ./Source/OpenEXR/OpenJPH/codestream/ojph_tile_comp.cpp
        ./Source/OpenEXR/OpenJPH/codestream/ojph_tile_comp.h
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_common.cpp
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_common.h
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_decoder.h
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_decoder32.cpp
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_decoder64.cpp
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_encoder.cpp
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_encoder.h
        ./Source/OpenEXR/OpenJPH/coding/ojph_block_encoder_avx2_apple.h
        ./Source/OpenEXR/OpenJPH/coding/table0.h
        ./Source/OpenEXR/OpenJPH/coding/table1.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_arch.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_arg.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_base.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_codestream.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_defs.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_file.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_mem.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_message.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_params.h
        ./Source/OpenEXR/OpenJPH/openjph/ojph_version.h
        ./Source/OpenEXR/OpenJPH/others/ojph_arch.cpp
        ./Source/OpenEXR/OpenJPH/others/ojph_file.cpp
        ./Source/OpenEXR/OpenJPH/others/ojph_mem.cpp
        ./Source/OpenEXR/OpenJPH/others/ojph_mem_c.c
        ./Source/OpenEXR/OpenJPH/others/ojph_message.cpp
        ./Source/OpenEXR/OpenJPH/shared/ojph_simd_vsx.h
        ./Source/OpenEXR/OpenJPH/transform/ojph_colour.cpp
        ./Source/OpenEXR/OpenJPH/transform/ojph_colour.h
        ./Source/OpenEXR/OpenJPH/transform/ojph_colour_local.h
        ./Source/OpenEXR/OpenJPH/transform/ojph_transform.cpp
        ./Source/OpenEXR/OpenJPH/transform/ojph_transform.h
        ./Source/OpenEXR/OpenJPH/transform/ojph_transform_local.h
    )
    include(cmake/OpenJPHSimd.cmake)
    set_property(SOURCE ${FreeImage_OPENJPH_SRCS} APPEND PROPERTY COMPILE_DEFINITIONS _FILE_OFFSET_BITS=64)
    set_source_files_properties(${FreeImage_ZSTD_SRCS} PROPERTIES COMPILE_DEFINITIONS ZSTD_DISABLE_ASM)
    set(FreeImage_OPENEXR_SRCS
        ./Source/OpenEXR/IexConfig.h
        ./Source/OpenEXR/IlmImf/ImfAcesFile.cpp
        ./Source/OpenEXR/IlmImf/ImfAcesFile.h
        ./Source/OpenEXR/IlmImf/ImfArray.h
        ./Source/OpenEXR/IlmImf/ImfAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfAttribute.h
        ./Source/OpenEXR/IlmImf/ImfAutoArray.h
        ./Source/OpenEXR/IlmImf/ImfB44Compressor.cpp
        ./Source/OpenEXR/IlmImf/ImfB44Compressor.h
        ./Source/OpenEXR/IlmImf/ImfBoxAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfBoxAttribute.h
        ./Source/OpenEXR/IlmImf/ImfBytesAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfBytesAttribute.h
        ./Source/OpenEXR/IlmImf/ImfCRgbaFile.cpp
        ./Source/OpenEXR/IlmImf/ImfCRgbaFile.h
        ./Source/OpenEXR/IlmImf/ImfChannelList.cpp
        ./Source/OpenEXR/IlmImf/ImfChannelList.h
        ./Source/OpenEXR/IlmImf/ImfChannelListAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfChannelListAttribute.h
        ./Source/OpenEXR/IlmImf/ImfCheckedArithmetic.h
        ./Source/OpenEXR/IlmImf/ImfChromaticities.cpp
        ./Source/OpenEXR/IlmImf/ImfChromaticities.h
        ./Source/OpenEXR/IlmImf/ImfChromaticitiesAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfChromaticitiesAttribute.h
        ./Source/OpenEXR/IlmImf/ImfColorMetadata.cpp
        ./Source/OpenEXR/IlmImf/ImfColorMetadata.h
        ./Source/OpenEXR/IlmImf/ImfCompositeDeepScanLine.cpp
        ./Source/OpenEXR/IlmImf/ImfCompositeDeepScanLine.h
        ./Source/OpenEXR/IlmImf/ImfCompression.cpp
        ./Source/OpenEXR/IlmImf/ImfCompression.h
        ./Source/OpenEXR/IlmImf/ImfCompressionAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfCompressionAttribute.h
        ./Source/OpenEXR/IlmImf/ImfCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfCompressor.h
        ./Source/OpenEXR/IlmImf/ImfCompressorDeepInternal.h
        ./Source/OpenEXR/IlmImf/ImfContext.cpp
        ./Source/OpenEXR/IlmImf/ImfContext.h
        ./Source/OpenEXR/IlmImf/ImfContextInit.cpp
        ./Source/OpenEXR/IlmImf/ImfContextInit.h
        ./Source/OpenEXR/IlmImf/ImfConvert.cpp
        ./Source/OpenEXR/IlmImf/ImfConvert.h
        ./Source/OpenEXR/IlmImf/ImfDeepCompositing.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepCompositing.h
        ./Source/OpenEXR/IlmImf/ImfDeepFrameBuffer.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepFrameBuffer.h
        ./Source/OpenEXR/IlmImf/ImfDeepImageState.h
        ./Source/OpenEXR/IlmImf/ImfDeepImageStateAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepImageStateAttribute.h
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineInputFile.h
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineInputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineInputPart.h
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineOutputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineOutputFile.h
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineOutputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepScanLineOutputPart.h
        ./Source/OpenEXR/IlmImf/ImfDeepTiledInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepTiledInputFile.h
        ./Source/OpenEXR/IlmImf/ImfDeepTiledInputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepTiledInputPart.h
        ./Source/OpenEXR/IlmImf/ImfDeepTiledOutputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepTiledOutputFile.h
        ./Source/OpenEXR/IlmImf/ImfDeepTiledOutputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfDeepTiledOutputPart.h
        ./Source/OpenEXR/IlmImf/ImfDoubleAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfDoubleAttribute.h
        ./Source/OpenEXR/IlmImf/ImfDwaCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfDwaCompressor.h
        ./Source/OpenEXR/IlmImf/ImfEnvmap.cpp
        ./Source/OpenEXR/IlmImf/ImfEnvmap.h
        ./Source/OpenEXR/IlmImf/ImfEnvmapAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfEnvmapAttribute.h
        ./Source/OpenEXR/IlmImf/ImfExport.h
        ./Source/OpenEXR/IlmImf/ImfFastHuf.cpp
        ./Source/OpenEXR/IlmImf/ImfFastHuf.h
        ./Source/OpenEXR/IlmImf/ImfFloatAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfFloatAttribute.h
        ./Source/OpenEXR/IlmImf/ImfFloatVectorAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfFloatVectorAttribute.h
        ./Source/OpenEXR/IlmImf/ImfForward.h
        ./Source/OpenEXR/IlmImf/ImfFrameBuffer.cpp
        ./Source/OpenEXR/IlmImf/ImfFrameBuffer.h
        ./Source/OpenEXR/IlmImf/ImfFramesPerSecond.cpp
        ./Source/OpenEXR/IlmImf/ImfFramesPerSecond.h
        ./Source/OpenEXR/IlmImf/ImfGenericInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfGenericInputFile.h
        ./Source/OpenEXR/IlmImf/ImfGenericOutputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfGenericOutputFile.h
        ./Source/OpenEXR/IlmImf/ImfHTCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfHTCompressor.h
        ./Source/OpenEXR/IlmImf/ImfHeader.cpp
        ./Source/OpenEXR/IlmImf/ImfHeader.h
        ./Source/OpenEXR/IlmImf/ImfHuf.cpp
        ./Source/OpenEXR/IlmImf/ImfHuf.h
        ./Source/OpenEXR/IlmImf/ImfIDManifest.cpp
        ./Source/OpenEXR/IlmImf/ImfIDManifest.h
        ./Source/OpenEXR/IlmImf/ImfIDManifestAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfIDManifestAttribute.h
        ./Source/OpenEXR/IlmImf/ImfIO.cpp
        ./Source/OpenEXR/IlmImf/ImfIO.h
        ./Source/OpenEXR/IlmImf/ImfInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfInputFile.h
        ./Source/OpenEXR/IlmImf/ImfInputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfInputPart.h
        ./Source/OpenEXR/IlmImf/ImfInputPartData.cpp
        ./Source/OpenEXR/IlmImf/ImfInputPartData.h
        ./Source/OpenEXR/IlmImf/ImfInputStreamMutex.h
        ./Source/OpenEXR/IlmImf/ImfInt64.h
        ./Source/OpenEXR/IlmImf/ImfIntAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfIntAttribute.h
        ./Source/OpenEXR/IlmImf/ImfKeyCode.cpp
        ./Source/OpenEXR/IlmImf/ImfKeyCode.h
        ./Source/OpenEXR/IlmImf/ImfKeyCodeAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfKeyCodeAttribute.h
        ./Source/OpenEXR/IlmImf/ImfLineOrder.h
        ./Source/OpenEXR/IlmImf/ImfLineOrderAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfLineOrderAttribute.h
        ./Source/OpenEXR/IlmImf/ImfLut.cpp
        ./Source/OpenEXR/IlmImf/ImfLut.h
        ./Source/OpenEXR/IlmImf/ImfMatrixAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfMatrixAttribute.h
        ./Source/OpenEXR/IlmImf/ImfMisc.cpp
        ./Source/OpenEXR/IlmImf/ImfMisc.h
        ./Source/OpenEXR/IlmImf/ImfMultiPartInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfMultiPartInputFile.h
        ./Source/OpenEXR/IlmImf/ImfMultiPartOutputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfMultiPartOutputFile.h
        ./Source/OpenEXR/IlmImf/ImfMultiView.cpp
        ./Source/OpenEXR/IlmImf/ImfMultiView.h
        ./Source/OpenEXR/IlmImf/ImfName.h
        ./Source/OpenEXR/IlmImf/ImfNamespace.h
        ./Source/OpenEXR/IlmImf/ImfOpaqueAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfOpaqueAttribute.h
        ./Source/OpenEXR/IlmImf/ImfOptimizedPixelReading.h
        ./Source/OpenEXR/IlmImf/ImfOutputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfOutputFile.h
        ./Source/OpenEXR/IlmImf/ImfOutputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfOutputPart.h
        ./Source/OpenEXR/IlmImf/ImfOutputPartData.cpp
        ./Source/OpenEXR/IlmImf/ImfOutputPartData.h
        ./Source/OpenEXR/IlmImf/ImfOutputStreamMutex.h
        ./Source/OpenEXR/IlmImf/ImfPartHelper.h
        ./Source/OpenEXR/IlmImf/ImfPartType.cpp
        ./Source/OpenEXR/IlmImf/ImfPartType.h
        ./Source/OpenEXR/IlmImf/ImfPixelType.h
        ./Source/OpenEXR/IlmImf/ImfPizCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfPizCompressor.h
        ./Source/OpenEXR/IlmImf/ImfPreviewImage.cpp
        ./Source/OpenEXR/IlmImf/ImfPreviewImage.h
        ./Source/OpenEXR/IlmImf/ImfPreviewImageAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfPreviewImageAttribute.h
        ./Source/OpenEXR/IlmImf/ImfPxr24Compressor.cpp
        ./Source/OpenEXR/IlmImf/ImfPxr24Compressor.h
        ./Source/OpenEXR/IlmImf/ImfRational.cpp
        ./Source/OpenEXR/IlmImf/ImfRational.h
        ./Source/OpenEXR/IlmImf/ImfRationalAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfRationalAttribute.h
        ./Source/OpenEXR/IlmImf/ImfRgba.h
        ./Source/OpenEXR/IlmImf/ImfRgbaFile.cpp
        ./Source/OpenEXR/IlmImf/ImfRgbaFile.h
        ./Source/OpenEXR/IlmImf/ImfRgbaYca.cpp
        ./Source/OpenEXR/IlmImf/ImfRgbaYca.h
        ./Source/OpenEXR/IlmImf/ImfRle.cpp
        ./Source/OpenEXR/IlmImf/ImfRle.h
        ./Source/OpenEXR/IlmImf/ImfRleCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfRleCompressor.h
        ./Source/OpenEXR/IlmImf/ImfScanLineInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfScanLineInputFile.h
        ./Source/OpenEXR/IlmImf/ImfSimd.h
        ./Source/OpenEXR/IlmImf/ImfStandardAttributes.cpp
        ./Source/OpenEXR/IlmImf/ImfStandardAttributes.h
        ./Source/OpenEXR/IlmImf/ImfStdIO.cpp
        ./Source/OpenEXR/IlmImf/ImfStdIO.h
        ./Source/OpenEXR/IlmImf/ImfStringAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfStringAttribute.h
        ./Source/OpenEXR/IlmImf/ImfStringVectorAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfStringVectorAttribute.h
        ./Source/OpenEXR/IlmImf/ImfSystemSpecific.cpp
        ./Source/OpenEXR/IlmImf/ImfSystemSpecific.h
        ./Source/OpenEXR/IlmImf/ImfTestFile.cpp
        ./Source/OpenEXR/IlmImf/ImfTestFile.h
        ./Source/OpenEXR/IlmImf/ImfThreading.cpp
        ./Source/OpenEXR/IlmImf/ImfThreading.h
        ./Source/OpenEXR/IlmImf/ImfTileDescription.h
        ./Source/OpenEXR/IlmImf/ImfTileDescriptionAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfTileDescriptionAttribute.h
        ./Source/OpenEXR/IlmImf/ImfTileOffsets.cpp
        ./Source/OpenEXR/IlmImf/ImfTileOffsets.h
        ./Source/OpenEXR/IlmImf/ImfTiledInputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfTiledInputFile.h
        ./Source/OpenEXR/IlmImf/ImfTiledInputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfTiledInputPart.h
        ./Source/OpenEXR/IlmImf/ImfTiledMisc.cpp
        ./Source/OpenEXR/IlmImf/ImfTiledMisc.h
        ./Source/OpenEXR/IlmImf/ImfTiledOutputFile.cpp
        ./Source/OpenEXR/IlmImf/ImfTiledOutputFile.h
        ./Source/OpenEXR/IlmImf/ImfTiledOutputPart.cpp
        ./Source/OpenEXR/IlmImf/ImfTiledOutputPart.h
        ./Source/OpenEXR/IlmImf/ImfTiledRgbaFile.cpp
        ./Source/OpenEXR/IlmImf/ImfTiledRgbaFile.h
        ./Source/OpenEXR/IlmImf/ImfTimeCode.cpp
        ./Source/OpenEXR/IlmImf/ImfTimeCode.h
        ./Source/OpenEXR/IlmImf/ImfTimeCodeAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfTimeCodeAttribute.h
        ./Source/OpenEXR/IlmImf/ImfVecAttribute.cpp
        ./Source/OpenEXR/IlmImf/ImfVecAttribute.h
        ./Source/OpenEXR/IlmImf/ImfVersion.cpp
        ./Source/OpenEXR/IlmImf/ImfVersion.h
        ./Source/OpenEXR/IlmImf/ImfWav.cpp
        ./Source/OpenEXR/IlmImf/ImfWav.h
        ./Source/OpenEXR/IlmImf/ImfXdr.h
        ./Source/OpenEXR/IlmImf/ImfZip.cpp
        ./Source/OpenEXR/IlmImf/ImfZip.h
        ./Source/OpenEXR/IlmImf/ImfZipCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfZipCompressor.h
        ./Source/OpenEXR/IlmImf/ImfZstdCompressor.cpp
        ./Source/OpenEXR/IlmImf/ImfZstdCompressor.h
        ./Source/OpenEXR/IlmThreadConfig.h
        ./Source/OpenEXR/OpenEXRConfig.h
        ./Source/OpenEXR/OpenEXRConfigInternal.h
    )
    # Include the runtime tables and their initialization helpers above,
    # while excluding standalone maintainer tools that define main().
        list(APPEND FreeImage_SOURCES ${FreeImage_IMATH_SRCS} ${FreeImage_IEX_SRCS}
            ${FreeImage_ILMTHREAD_SRCS} ${FreeImage_OPENEXRCORE_SRCS}
            ${FreeImage_LIBDEFLATE_SRCS} ${FreeImage_OPENEXR_SRCS}
            ${FreeImage_ZSTD_SRCS} ${FreeImage_OPENJPH_SRCS})
        include_directories(Source/OpenEXR)
        include_directories(Source/OpenEXR/Half)
        include_directories(Source/OpenEXR/Iex)
        include_directories(Source/OpenEXR/IlmImf)
        include_directories(Source/OpenEXR/IlmThread)
        include_directories(Source/OpenEXR/OpenEXRCore)
        include_directories(Source/Imath)
        include_directories(Source/OpenEXR/Zstd/lib)
        include_directories(Source/OpenEXR/OpenJPH)
        include_directories(Source/OpenEXR/OpenJPH/openjph)
        include_directories(Source/LibDeflate)
        add_definitions(-DINCLUDE_LIB_OPENEXR)

        # Same clang-cl vs real-MSVC target-feature gating issue as LibWebP
        # above: OpenEXR's own runtime-dispatched SSE2/SSSE3/SSE4.1/AVX2
        # paths (ImfZip.cpp, internal_zip.c, internal_dwa.c, unpack.c, ...)
        # compile unconditionally, real MSVC doesn't need /arch: for them,
        # clang-cl does.
        if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set_source_files_properties(${FreeImage_OPENEXR_SRCS} ${FreeImage_OPENEXRCORE_SRCS}
                PROPERTIES COMPILE_OPTIONS "/clang:-mssse3;/clang:-msse4.1;/clang:-mavx;/clang:-mavx2")
        endif()
    endif()
endif()

# Need include directories for FreeImage itself. Per-sub-lib include dirs
# (bundled Source/Lib* vs. system) are added above, inside each BUILD_*
# block, so a system-lib build doesn't also see the bundled headers.
if(BUILD_OPENEXR AND USE_SYSTEM_OPENEXR)
    # Source/ also contains Imath/. Do not let that shadow installed
    # <Imath/...> headers and mix ABI namespaces with system OpenEXR.
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/core-headers")
    file(GLOB FreeImage_CORE_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")
    foreach(_freeimage_header IN LISTS FreeImage_CORE_HEADERS)
        get_filename_component(_freeimage_header_name "${_freeimage_header}" NAME)
        configure_file("${_freeimage_header}"
            "${CMAKE_CURRENT_BINARY_DIR}/core-headers/${_freeimage_header_name}" COPYONLY)
    endforeach()
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/core-headers")
else()
    include_directories(Source)
endif()
include_directories(Source/Metadata)
include_directories(Source/FreeImageToolkit)

# Generate the Windows version resource from the git-tag-derived version
# computed above, so FILEVERSION/PRODUCTVERSION (and the copyright year)
# never need a manual bump again. Only meaningful on Windows, but
# configure_file() is cheap enough to just always run it.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/FreeImage.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/FreeImage.rc"
    @ONLY
)
if(WIN32)
    list(APPEND FreeImage_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/FreeImage.rc")
endif()

if(FREEIMAGE_STATIC)
    # Define the FreeImage library
    add_library(${PROJECT_NAME} STATIC
        ${FreeImage_SOURCES}
)
else()
    add_library(${PROJECT_NAME} SHARED
        ${FreeImage_SOURCES}
)
endif()

if (DEFINED FreeImage_LIBS)
    if(FREEIMAGE_LINUX_PACKAGE_LAYOUT)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${FreeImage_LIBS})
    else()
        target_link_libraries(${PROJECT_NAME} ${FreeImage_LIBS})
    endif()
endif()

# Public header for add_subdirectory() and the installed package. Copying
# just FreeImage.h keeps Source/'s internal headers off the consumer's
# include path. BUILD_INTERFACE is the build tree; INSTALL_INTERFACE
# matches install(FILES Source/FreeImage.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}).
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/FreeImage.h"
    "${CMAKE_CURRENT_BINARY_DIR}/include/FreeImage.h"
    COPYONLY
)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC
    DISABLE_PERF_MEASUREMENT
    $<$<NOT:$<BOOL:${USE_SYSTEM_LIBOPENJPEG}>>:OPJ_STATIC>
    NO_LCMS
    NO_WINDOWS
)
# Static consumers must see this or FreeImage.h uses dllimport for the
# symbols they are linking directly. Directory add_definitions() does not
# propagate out of add_subdirectory().
if(FREEIMAGE_STATIC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC FREEIMAGE_LIB)
elseif(WIN32)
    # Shared Windows builds (MSVC and MinGW, including llvm-mingw) compile
    # this target with dllexport. Consumers do not get the define, so
    # FreeImage.h gives them dllimport. Without it the library's own
    # objects are built dllimport and the link fails (issue #127).
    target_compile_definitions(${PROJECT_NAME} PRIVATE FREEIMAGE_EXPORTS)
endif()


# Apple-specific settings
if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO")
    if(BUILD_PLATFORM_APPLE_IOS)
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
    endif()
endif()

# Enable Multi-core compilation in MSVC (not supported by clang-cl)
if (MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
endif()
if (WIN32)
    # Add ws2_32.lib for Windows builds
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

if(FREEIMAGE_LINUX_PACKAGE_LAYOUT)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR FREEIMAGE_STATIC)
        message(FATAL_ERROR "FREEIMAGE_LINUX_PACKAGE_LAYOUT requires a Linux shared build")
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME freeimage VERSION ${PROJECT_VERSION} SOVERSION 3)
    configure_file(cmake/freeimage.pc.in freeimage.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/freeimage.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT Development)
    install(FILES license-fi.txt license-gplv2.txt
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/freeimage-maintained
        COMPONENT Runtime)
    install(FILES Source/OpenEXR/Copyrights/ilmbase/LICENSE
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/freeimage-maintained
        RENAME OpenEXR-Half-LICENSE COMPONENT Runtime)
endif()

# Test suite
if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(TestAPI)
endif ()

# Install the library. Runtime is the shared library (DLL / .so / .dylib).
# Development is the static archive or import library, the public header,
# and the CMake package files. `cmake --install . --component Runtime`
# and `--component Development` select one group.
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
        NAMELINK_COMPONENT Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Development
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES Source/FreeImage.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Development
)

# Install the CMake Targets
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    COMPONENT Development
)

# Install the CMake config
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    COMPONENT Development
)
