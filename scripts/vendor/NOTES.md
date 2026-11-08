# Vendored codec integration

Current vendored releases (issue #137, updated for issue #148):

| Library | Upstream tag | Commit | Library sources |
|---|---|---|---|
| libdeflate | `v1.26` | `92e6a0db9fa848d742f9eb286c92afc60f2c3dda` | `common_defs.h`, `libdeflate.h`, `lib/` |
| libtiff | `v4.7.2` | `d01a94be176f5f6a87f7ee1c0b32e65416aa2b4d` | `libtiff/`, plus `port/libport.h` |
| OpenEXR | `v3.5.3` | `3950290530c4fd4776667d0dc4d65a9d003eb936` | `src/lib/{Iex,IlmThread,OpenEXR,OpenEXRCore}` |

OpenEXR's `OpenEXR` library directory maps to `Source/OpenEXR/IlmImf`.
Its installed-style `Imath/` include prefixes are removed because this fork
keeps Imath headers flat under `Source/Imath`. Keep the checked-in portable
configuration headers, update version numbers and internal namespaces together,
and preserve the include-order guards in `OpenEXRCore/openexr_version.h`.
The upstream ARM64EC SIMD guards are now present in 3.5.1.

OpenEXR's auxiliary Zstandard 1.5.7 snapshot comes from that same tag's
`external/` tree. OpenJPH was updated independently from 0.32.0 to 0.33.0
(345a8bc; upstream tag `0.33.0`) for #259, copying `src/core/`, the README,
and license from the upstream release. CMake and GNU/macOS
make enable OpenJPH's runtime-dispatched x86 kernels by default. Each SSE,
SSE2, SSSE3, AVX, AVX2, and AVX512 source group gets its own compiler flags;
scalar and CPU-detection sources keep the baseline ISA. Unsupported compiler
flags omit that group and disable its dispatch references. CMake handles MSVC,
clang-cl, cross targets, and macOS universal builds. The header opt-in and
actual compiler target checks keep ARM64/ARM64EC and other targets portable;
this snapshot has no ARM SIMD kernels. VSX and WASM kernels are not integrated.

Use `-DFREEIMAGE_OPENJPH_SIMD=OFF` with CMake or `FREEIMAGE_OPENJPH_SIMD=0`
with make for a portable build. Use a separate build directory (or clean make
objects) when changing the option. The C allocation helper uses its upstream
portable aligned-allocation fallback. Zstandard uses its C implementation
with `ZSTD_DISABLE_ASM`; preserve the ARM64EC exclusions in
`common/{compiler.h,cpu.h,portability_macros.h}` so `_M_X64` does not select
x86 intrinsics or CPUID on ARM64EC.

The Linux ARM CPU detector requests `_GNU_SOURCE` before system headers so
`O_CLOEXEC` is available under strict C11, including the GNU make path.

libdeflate 1.26 requires C11 (MSVC 2019 16.8 or later). CMake applies C11
specifically to its sources; GNU make defaults to C11. Upstream now excludes ARM64EC
from x86 detection and uses its portable fallback there.

libtiff retains FreeImage's checked-in `tif_config.h` and `tiffconf.h`;
`tiffvers.h` is substituted from the upstream template. Preserve these local
patches when re-importing:

- `tif_predict.c` excludes ARM64EC from the `_M_X64` SSE guards.
- `tif_win32.c` omits allocation/memory helpers supplied by `PluginTIFF.cpp`.
- `tif_config.h` uses `"zu"` for `size_t` and `PRId64` for this fork's
  `int64_t` signed size type, without a leading `%`.

The full 4.7.2 sync moves cursor fields into `TIFFDirectory`; the G3 loader
uses `tif_dir.td_row`. PixarLog now includes the decompression-ratio hook.

Always reconcile upstream CMake library source lists with both FreeImage
build paths. Include generated tables and their initialization helpers,
but exclude table-generator executables (`b44ExpLogTable.cpp`, `dwaLookups.cpp`).
After an update, run TestAPI and EXR/TIFF/G3 round trips, including EXR's new
compression methods and variable-sample deep ZSTD data, with AddressSanitizer.
Windows, ARM64EC, and Linux GNU make validation belongs in the existing CI matrix.

OpenEXRCore ZSTD uses posix_memalign on Android below API 28, where
aligned_alloc is unavailable. Preserve its 64-byte alignment, overflow
checks, and matching free cleanup when refreshing the vendor snapshot.

OpenJPH's AVX2 encoder includes the upstream MSVC 2022 Win32 Debug
compiler-crash workaround from [OpenJPH #395](https://github.com/aous72/OpenJPH/pull/395).
Copy the referenced previous context value into a local before inserting it
into the SIMD vector. OpenJPH 0.33.0 includes this workaround upstream.

libpng 1.6.59 comes from v1.6.59 (cd952f49f95bb27154ae77dbb103032d95f6e580).
Its prebuilt pnglibconf.h changes only the version comment; retain this fork's
configuration. Library source lists are unchanged for both codec updates.

OpenEXR 3.5.2 introduces OpenJPH NLT lookup tables for lossy LJ2K and a
compatibility fallback for files written by 3.5.0/3.5.1 without the tables.
Regenerate those older lossy LJ2K files for best results, per upstream notes.
At the 3.5.2 update, the Zstandard and OpenJPH snapshots were unchanged.

OpenEXRCore's Windows file backend uses CreateFileW unconditionally. This was
a local patch until upstream merged [OpenEXR #2712](https://github.com/AcademySoftwareFoundation/openexr/pull/2712)
in 3.5.3, which now ships the same code, so it no longer needs preserving.

OpenEXR 3.5.3 adds its own posix_memalign fallback in OpenEXRCore ZSTD for
macOS deployment targets below 10.15. It sits beside the Android fallback
above; keep both. The 3.5.3 update was applied as the upstream
v3.5.2..v3.5.3 diff of the four library directories, with no new or removed
source files. Zstandard was unchanged by the 3.5.3 update. OpenJPH stayed at
0.32.0 for that update and moved to 0.33.0 separately; see below.

OpenJPH 0.33.0 also includes the SSE memory-clear fix upstream, using
_mm_setzero_ps/_mm_storeu_ps rather than SSE2 integer intrinsics for i686.
The remaining local changes are the FreeImage SIMD opt-in/ARM64EC guards
in `ojph_arch.h` and removal of the unconditional `_BitScanForward64`
intrinsic pragma (unavailable on Win32). No library sources were added or
removed, so both build paths keep the same source lists.

The NLT LUT setter in 0.33.0 adds defaulted encoder options; OpenEXR 3.5.3
remains source compatible and uses upstream's precise inverse-LUT default.
`OpenJphNltTest` covers the 8-bit LUT read overflow and truncated markers;
`VendorCodecTest` exercises HTJ2K/LJ2K EXR round trips.
