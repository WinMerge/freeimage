# Changelog

Release notes for this fork, newest first. The 3.19 line starts from upstream FreeImage 3.19.0 on SVN.

## Unreleased

- Fix the OpenJPH SSE memory-clear kernel's accidental SSE2 requirement on 32-bit GCC builds.
- Add MinGW32 GCC CI for static and shared libraries, with OpenEXR/SIMD enabled and an explicit SSE-without-SSE2 compile check.

## 3.19.21 — Unreleased

- **libpng 1.6.58 → 1.6.59.** Includes the upstream fix for CVE-2026-46675 (use-after-free following incomplete compressed metadata), restores correctly positioned hIST chunks, and avoids double frees after allocation failure.
- **OpenEXR 3.5.1 → 3.5.2.** Adds OpenJPH nonlinear lookup tables for lossy LJ2K and a compatibility fallback for older files. Upstream recommends regenerating lossy LJ2K files written by 3.5.0/3.5.1 for best results. Zstandard and OpenJPH versions remain unchanged.
- OpenEXR's Windows file backend uses CreateFileW instead of CreateFile2, removing that Windows 8 API dependency for older Windows runtimes.
- Apply the upstream OpenJPH AVX2 encoder workaround for MSVC 2022 Win32 Debug compiler crashes.
- Add Win32 MSVC and clang-cl CI in Debug and Release, including codec round trips and Windows API import checks.
- Add CR3 and X3F RAW-format support and fix CR3 extension detection.

## 3.19.20 — 4 Oct 2026

Linux ARM and Android compatibility fixes for the bundled codecs.

- Libdeflate's Linux ARM CPU detector requests GNU feature declarations before system headers, keeping `O_CLOEXEC` available under strict C11.
- OpenEXR ZSTD uses `posix_memalign()` below Android API 28, preserving 64-byte buffer alignment and supporting API 25 targets.
- Add a Linux ARM64 cross-build job with bundled codecs and libdeflate's strict C11 settings.
- Add Android API 25 CI for ARMv7 and ARM64. Both jobs compile and link FreeImage and its test executables; runtime execution requires a device or emulator.
- Vendored library versions are unchanged from 3.19.19.

## 3.19.19 — 3 Oct 2026

Bundled codec updates, OpenJPH SIMD, and loader/build fixes.

- **OpenEXR 3.3.14 → 3.5.1**, including upstream-vendored **Zstandard 1.5.7** and **OpenJPH 0.32.0**. Add round-trip coverage for flat EXR ZSTD and HTJ2K/LJ2K, plus deep EXR ZSTD.
- **Libdeflate → 1.26** with a complete source sync and C11 compilation for its bundled sources.
- **LibTIFF 4.7.1 → 4.7.2** with a complete library sync. Update raw G3 fax decoding for the TIFF directory cursor change and use portable size format strings.
- Enable runtime-dispatched OpenJPH x86 SIMD in CMake and GNU/macOS make. Compile kernels with separate ISA flags; ARM and ARM64EC use the portable implementation. CMake can disable it with `-DFREEIMAGE_OPENJPH_SIMD=OFF`; make accepts `FREEIMAGE_OPENJPH_SIMD=0`.
- Fix BMP loading with extended info headers.
- Keep bundled WebP and libdeflate on their ARM paths for ARM64EC; add ARM64 and ARM64EC Windows cross-build CI.
- Synchronize and deduplicate legacy make sources. Add Linux ASan codec tests with SIMD enabled and disabled.

## 3.19.18 — 28 Sep 2026

LibRaw is on by default. No new CVEs.

- **LibRaw 0.22.2** is included unless the build passes `-DBUILD_LIBRAWLITE=OFF`. MSVC and clang-cl compile it with `LIBRAW_NODLL`, so the bundled sources are not `dllimport` (C2491).
- `openexr_version.h` no longer redefines `OPENEXR_VERSION_PATCH` after `OpenEXRConfig.h` (MSVC C4005). The fallback in that header is 3.3.14.
- A weekly GitHub Action compares vendored `Source/` versions with upstream releases and opens one issue when any library is newer.
- `CHANGELOG.md` covers this fork from upstream FreeImage 3.18.0 through 3.19.17, including the SVN 3.19.0 merge.

## 3.19.17 — 24 Sep 2026

CMake and JPEG-XR build fix. No new CVEs.

- JPEG-XR is built on Windows again. The old CMake default never enabled it, and the source list was cut off, so the plugin did not link.
- `JXRMeta.h` no longer needs the removed MSVC `__in` annotation.
- MinGW uses the system `guiddef.h` and `_byteswap_ulong`.
- `add_subdirectory()` gets an include path for `FreeImage.h`.
- A static build publishes `FREEIMAGE_LIB`. A shared Windows build compiles with `FREEIMAGE_EXPORTS`.
- `cmake --install` supports `--component Runtime` and `--component Development`.
- iOS builds use `CMAKE_SYSTEM_NAME=iOS` with the Xcode generator. See `README.iphone`.

## 3.19.16 — 22 Sep 2026

Security release for the bundled decoders.

- **OpenEXR 3.3.13 → 3.3.14.** [GHSA-rmgv-rm47-38g3](https://github.com/AcademySoftwareFoundation/openexr/security/advisories/GHSA-rmgv-rm47-38g3) and [GHSA-5j5m-22wr-mhc6](https://github.com/AcademySoftwareFoundation/openexr/security/advisories/GHSA-5j5m-22wr-mhc6) (High). Unbounded allocation while parsing an `idmanifest`. Upstream had requested CVEs. None were named in the release.
- **LibRaw 0.22.1 → 0.22.2.** Allocation overflows and missing checks in CRX, X3F, Bayer, Olympus 14-bit, floating-point DNG, and thumbnails. No CVE ids from upstream.
- **LibTIFF stays 4.7.1.** PixarLog and OJPEG only take the 4.7.2 integer-overflow hardening. The rest of 4.7.2 was not taken.
- **Imath 3.1.12 → 3.2.3.**
- Libdeflate compress and decompress paths were synced toward 1.26 for an output-overflow fix. The header string is still `1.18`.
- OpenEXR builds on ARM64EC. MSVC was including x86 intrinsic headers there.

## 3.19.15 — 14 Sep 2026

Three old FreeImage loader overflows. All are NVD 8.8 High.

- **CVE-2020-24292.** ICO `LoadStandardIcon()`. Illegal bits-per-pixel or dimensions walked off the heap.
- **CVE-2020-24293.** PSD thumbnail packed-row copy used a separate `WidthBytes` from the allocated scanline.
- **CVE-2020-24295.** PSD `ReadImageLine()` wrote a channel past the DIB on both raw and RLE paths.

## 3.19.14 — 2 Sep 2026

The remaining FreeImage-owned CVEs from issue #35.

- **CVE-2024-9029** and **CVE-2024-28568.** IPTC 1-byte out-of-bounds read.
- **CVE-2024-28570.** Exif MakerNote signature compared past a short tag.
- **CVE-2024-28573** and **CVE-2024-28577.** Exif APP1 read with no length check.
- **CVE-2024-28578** and **CVE-2024-28580.** RAS loop counters and allocation overflow.
- **CVE-2024-28583.** XBM off-by-one stack write.
- **CVE-2024-28566.** TIFF `AssignPixel()` copied past the source strip.
- **CVE-2024-28571.** JPEG `fill_input_buffer()` use-after-free.
- **CVE-2024-28572.** Canon MakerNote tags read as the wrong type.
- XPM short pixel rows and color-name `sprintf` were hardened in the same change. No separate CVE id.

## 3.19.13 — 2 Sep 2026

Speed, a TIFF crash, and the big vendor bump.

- Sequential animated GIF playback is about 8× faster. Issue #18.
- Heap use-after-free when a TIFF has both an EXIF IFD and an ICC profile. Issue #27.
- **LibRaw 0.22.1, libwebp 1.6.0, libpng 1.6.58.**
- **OpenEXR 3.3.13** closes **CVE-2024-28562**, **CVE-2024-28563**, **CVE-2024-28564**, and **CVE-2024-28569**.
- **OpenJPEG 2.5.4** closes **CVE-2024-28574**, **CVE-2024-28575**, and **CVE-2024-28576**.
- libtiff and IJG libjpeg were bumped in the same stack. The current tree is libtiff 4.7.1 and libjpeg 10 (25 Jan 2026).
- OpenEXR is on by default for C++17 and newer. clang-cl, VS 2022/2026 presets, and MSYS2 MINGW64/UCRT64/CLANG64 were added.

## 3.19.12 — 17 Aug 2026

Loader security fixes and zlib 1.3.2.

- **CVE-2020-22524.** PFM integer overflow, undersized allocation.
- **CVE-2020-24294**, **CVE-2024-28565**, and **CVE-2025-65803.** PSD RLE unpacker heap overflow.
- **CVE-2024-28579** and **CVE-2024-28582.** HDR RLE scanline heap overflow.
- **CVE-2024-28584.** JPEG 2000 NULL pointer dereference.
- **CWE-457.** PCX uninitialized read when `planes=3` and `bpp=8`.
- **CWE-787.** PICT `UnpackBits` heap overflow.
- BMP RLE8 no longer rejects valid files. PICT and RAS crash fixes.
- **zlib 1.3.2.** Optional system-library linking via `USE_SYSTEM_*`.

## 3.19.11 — 22 Apr 2026

Three out-of-bounds writes found in academic testing. The release did not assign CVE ids.

- **CWE-787.** Crafted PICT file.
- **CWE-787.** 24-byte CUT file (`width=0x230B`). Crash in `PluginCUT.cpp`.
- **CWE-122.** 21-byte TGA file.
- Hotfixes for the macOS zlib and libpng builds.

## 3.19.10 — 31 Dec 2024

Same vendor set as 3.19.9. **LibTIFF 4.6.1.** C++23 support. `wchar` path fixes. `toHalfFloat` fix. No 32-bit DLL.

## 3.19.9 — 30 Jul 2024

Same notes as 3.19.8. **LibTIFF 4.6.1**, zlib 1.2.13, libpng 1.6.41, libwebp 1.2.1, LibRaw 0.21.1, libjpeg 9d, jxrlib 1.2.

## 3.19.8 — 29 Jul 2024

**LibTIFF 4.5.0 → 4.6.1.** `toHalfFloat` fix. Otherwise the same CMake and vendor set as 3.19.7.

## 3.19.7 — 26 Jul 2024

CMake build, C++23, and the first bundled set: zlib 1.2.13, libpng 1.6.41, LibTIFF 4.5.0, libwebp 1.2.1, LibRaw 0.21.1, libjpeg 9d, jxrlib 1.2. `wchar` path fixes. No 32-bit DLL.

## 3.19.6 — 10 Jul 2024

Unicode `wchar` path fixes. No separate GitHub release text. No CVEs.

## 3.19.5, 3.19.4, 3.19.3, 3.19.2 — 9–10 Jul 2024

The first CMake releases of this fork after the 3.19.0 SVN merge. All four published the same vendor list: zlib 1.2.13, libpng 1.6.41, LibTIFF 4.5.0, libwebp 1.2.1, LibRaw 0.21.1, libjpeg 9d, jxrlib 1.2. C++23 support. No 32-bit DLL. No CVEs in the release text.

## 3.19.0 — 24 Dec 2023

Merged in the changes from SVN FreeImage 3.19.0.

Upstream library set: zlib 1.2.13, libpng 1.6.39, LibTIFF 4.5.0 (patch 2023-05-18), libwebp 1.2.1 (patch 2021-10-29), LibRaw 0.21.1 (patch 2023-05-13), libjpeg 9d, jxrlib 1.2. No 32-bit DLL or static library. Visual Studio 2013 project files removed.

Also from that SVN drop: Exif 2.3.2 tags and an `ExtraSamples` metadata key, better float TIFF compression, TIFF no longer treats Canon CR2 as TIFF, `TIFF_DEFLATE` saves as `TIFF_ADOBE_DEFLATE`, and malicious-image hardening in TIFF, BMP, PFM, JXR, TGA, DDS, PCX, PNM, and JPEG 2000. This fork's tag also notes ARM64EC via CMake and an Emscripten fix. No CVE ids in the release text.

## 3.18.10 — 29 Jun 2023

PluginTIFF fix.

## 3.18.9 — 11 Jun 2023

CMake can leave out LibJXR and LibWebP.

## 3.18.8 — 11 Jun 2023

CMake fix.

## 3.18.7 — 3 May 2023

LibJXR moved behind a CMake option. Building extended JPEG on Windows was pulling it in unconditionally.

## 3.18.6 — 28 Apr 2023

JXR builds on non-Windows.

## 3.18.5 — 25 Apr 2023

LibJXR bad include fixed.

## 3.18.4 — 17 Apr 2023

CMake source and header list filled in.

## 3.18.3 — 17 Apr 2023

libpng NEON disabled.

## 3.18.2 — 16 Apr 2023

CMakeLists update.

## 3.18.1 — 16 Apr 2023

`cmake_minimum_required` added. First CMake cut of the 3.18 tree.

## 3.18.0 — 31 Jul 2018

Upstream FreeImage 3.18.0. This fork's `3.18.0` tag is the 14 Jul 2021 Android Makefile commit on that tree.

Vendors: zlib 1.2.11, LibRaw 0.19, libwebp 1.0.0, libpng 1.6.35, LibTIFF 4.0.9, OpenEXR 2.2.1, libjpeg 9c. Visual Studio 2017 projects added. Visual Studio 2005 and 2008, `FreeImage_RotateClassic`, and the internal deprecation manager removed.

Also: PSD saving, 32-bit CMYK JPEG saving, `FreeImage_Validate`, DDS 16-bit uncompressed RGB loaded as 24-bit, palette images with transparency in `FreeImage_ConvertToRawBits`, and malicious-image fixes for PCX and XPM. No CVE ids in the release text.
