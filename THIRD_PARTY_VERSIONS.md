# Third-party (vendored) library versions

Tracks what's actually bundled under `Source/` against upstream, so drift gets
caught before it turns into a CVE backlog or a silent ARM64EC-class bug.

**Update this before every release** (same step as bumping
`FREEIMAGE_RELEASE_SERIAL` in `Source/FreeImage.h`, see
[danoli3/FreeImage#112](https://github.com/danoli3/FreeImage/pull/112)) and
whenever a vendored library is re-synced. A weekly GitHub Action
(`.github/workflows/upstream-versions.yml`) opens an issue when one of
these libraries has a newer stable upstream release.

| Library | Path | Vendored | Latest upstream | Status | Version source |
|---|---|---|---|---|---|
| OpenEXR | `Source/OpenEXR` | 3.5.2 | 3.5.2 (2026-10-02) | up to date; includes upstream ARM64EC guards | `OpenEXRConfig.h` |
| Imath | `Source/Imath` | 3.2.3 | 3.2.3 (2026-08-20) | up to date | `ImathConfig.h` |
| LibDeflate | `Source/LibDeflate` | 1.26 | 1.26 (2026-08-22) | up to date; requires C11 | `libdeflate.h` |
| LibJPEG (IJG) | `Source/LibJPEG` | 10.0 (25-Jan-2026) | 10.0 (25-Jan-2026) | up to date | `jversion.h` |
| LibOpenJPEG | `Source/LibOpenJPEG` | 2.5.4 | 2.5.4 | up to date | `opj_config_private.h` |
| LibPNG | `Source/LibPNG` | 1.6.59 | 1.6.59 (2026-09-28) | up to date | `png.h` |
| LibRawLite | `Source/LibRawLite` | 0.22.2 | 0.22.2 (2026-07-16) | up to date | `libraw/libraw_version.h` |
| LibTIFF4 | `Source/LibTIFF4` | 4.7.2 | 4.7.2 (2026-06-27) | up to date; full library sync | `tiffvers.h` |
| LibWebP | `Source/LibWebP` | 1.6.0 (30-Jun-2025) | 1.6.0 | up to date | `NEWS` |
| LibJXR | `Source/LibJXR` | unversioned jxrlib snapshot | n/a — unmaintained (last active fork: [4creators/jxrlib](https://github.com/4creators/jxrlib)) | no upstream to track | `README` |
| ZLib | `Source/ZLib` | 1.3.2 | 1.3.2 | up to date | `zlib.h` |

OpenEXR, LibDeflate, and LibTIFF4 re-synced against their upstream release
tags on 2026-10-02 for issue #137. All vendored version entries, including
OpenEXR's auxiliary codecs below, were rechecked against their source headers
on 2026-10-16 for FreeImage 3.19.24. Vendored versions are unchanged from
3.19.23. The HEIF/AVIF and JPEG XL integrations use optional system
libraries; they do not add bundled codec snapshots. The latest-upstream
column retains the previous release review; this check verifies the bundled
versions rather than asserting a fresh upstream-release audit.

OpenEXR 3.5.2 also brings these upstream-vendored auxiliary codecs:

| Library | Path | Vendored | Version source |
|---|---|---|---|
| Zstandard | `Source/OpenEXR/Zstd` | 1.5.7 | `lib/zstd.h` |
| OpenJPH | `Source/OpenEXR/OpenJPH` | 0.32.0 | `openjph/ojph_version.h` |

These snapshots follow OpenEXR's release rather than being updated independently.
See [vendor integration notes](scripts/vendor/NOTES.md) for source provenance,
configuration choices, and local patches to preserve in the next update.

OpenEXR 3.5.2 and libpng 1.6.59 re-synced on 2026-10-08 for issue #148.
OpenEXR's auxiliary codec snapshots are unchanged from 3.5.1.

Optional system-only codec: `BUILD_LIBHEIF=ON` requires libheif >= 1.12
(LGPL-3.0-or-later) plus HEVC/AV1 decoder backends. It is not vendored and is
disabled by default. Backend versions and licenses depend on the installation.
The HEIF/AVIF plugin adapts FreeImageRe's FIPL/GPLv2-licensed decoder at commit
`b51d3945bfe785ad382a0969f1376aee344af8c6`; see its source header and README.

Optional system-only JPEG XL codec: BUILD_LIBJXL=ON requires libjxl >=0.7.
It is disabled by default; no libjxl, Brotli, Highway or CMS sources are bundled
by this integration. Their versions follow the installed system packages.
