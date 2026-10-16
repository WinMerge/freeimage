# Optional HEIF/AVIF and JPEG XL source bundles

These are pristine upstream source archives, checked into this repository so
normal and source-release builds work offline with CMake 3.20 or newer. Archives remain pristine. A local CMake patch guards libjxl's optional
tcmalloc pkg-config probe when pkg-config is disabled; see
`cmake/patches/libjxl-no-pkg-config.cmake`. CMake verifies SHA-256 before
extracting into the build directory. No git submodules, SDK installation,
pkg-config, or configure-time downloads are needed for the bundled path.
`sources.json` records exact versions, source URLs and SHA-256 digests.
The full source/license notices remain in each archive; copies of the main
license and patent notices are in `licenses/` and installed with FreeImage.

| Feature | Sources | Licenses (see full texts) |
| --- | --- | --- |
| HEIF container | libheif 1.23.5 | LGPL-3.0-or-later |
| HEVC decode | libde265 1.1.3 | LGPL-3.0-or-later |
| HEVC encode | Kvazaar 2.3.2 | BSD-3-Clause |
| AV1 decode/encode | libaom 3.15.1 | BSD-2-Clause, patent grant |
| JPEG XL | libjxl 0.12.0 | BSD-3-Clause, patent grant |
| JPEG XL compression | Brotli 1.2.0 | MIT |
| JPEG XL SIMD | Highway 1.4.0 | Apache-2.0 or BSD-3-Clause |
| JPEG XL color management | Little CMS 2.19.1 | MIT |

FreeImage's own FIPL/GPL licensing does not replace these dependency licenses.
The bundled codec archives are compiled as position-independent static
libraries, including when FreeImage itself is shared. Distributors must
account for the LGPL requirements for libheif/libde265, including applicable
source and relinking obligations. Selecting `USE_SYSTEM_LIBHEIF=ON` retains
the system SDK/dynamic-library approach.

HEIF backends are built in rather than dynamically loaded plugins. No x265,
x264, dav1d or SVT-AV1 dependency is added. AOM and libde265 use portable code
to avoid requiring NASM or assuming host SIMD features in cross builds;
Highway retains JPEG XL's runtime-dispatched SIMD support.

Kvazaar supports lossy HEIF output, including alpha and metadata, but the
bundled backend cannot provide exact RGB/4:4:4 lossless HEIF. `HEIF_LOSSLESS`
therefore returns FALSE for bundled HEIF rather than silently losing pixels.
AVIF lossless through AOM and JPEG XL lossless remain supported. For exact
lossless HEIF, use a system libheif with a suitable encoder such as x265.

Build integration is in `cmake/FreeImageBundledCodecs.cmake`. Each upstream
CMake build is isolated, so its options and required language standard do not
change FreeImage's public build settings (libheif requires C++20, libjxl C++17).

To update a snapshot, download the upstream release URL, verify its provenance,
replace the archive, update `sources.json`, the SHA-256 in the CMake module,
this table, license copies and `THIRD_PARTY_VERSIONS.md`, then run bundled
static/shared, installed-consumer, system-codec, x86 and Android API 25 CI.
The existing hand-maintained Makefiles continue to build unsupported stubs for
these optional codecs; enable them through CMake.
