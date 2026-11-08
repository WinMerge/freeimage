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
| OpenEXR | `Source/OpenEXR` | 3.5.3 | 3.5.3 (2026-10-31) | up to date; includes upstream ARM64EC guards | `OpenEXRConfig.h` |
| Imath | `Source/Imath` | 3.2.3 | 3.2.3 (2026-08-20) | up to date | `ImathConfig.h` |
| LibDeflate | `Source/LibDeflate` | 1.26 | 1.26 (2026-08-22) | up to date; requires C11 | `libdeflate.h` |
| LibJPEG (IJG) | `Source/LibJPEG` | 10.0 (25-Jan-2026) | 10.0 (25-Jan-2026) | up to date | `jversion.h` |
| LibOpenJPEG | `Source/LibOpenJPEG` | 2.5.4 | 2.5.4 | up to date | `opj_config_private.h` |
| LibPNG | `Source/LibPNG` | 1.6.59 | 1.6.59 (2026-09-28) | up to date | `png.h` |
| LibRawLite | `Source/LibRawLite` | 0.22.3 | 0.22.3 (2026-10-24) | up to date | `libraw/libraw_version.h` |
| LibTIFF4 | `Source/LibTIFF4` | 4.7.2 | 4.7.2 (2026-06-27) | up to date; full library sync | `tiffvers.h` |
| LibWebP | `Source/LibWebP` | 1.6.0 (30-Jun-2025) | 1.6.0 | up to date | `NEWS` |
| LibJXR | `Source/LibJXR` | unversioned jxrlib snapshot | n/a — unmaintained (last active fork: [4creators/jxrlib](https://github.com/4creators/jxrlib)) | no upstream to track | `README` |
| ZLib | `Source/ZLib` | 1.3.2 | 1.3.2 | up to date | `zlib.h` |

All vendored versions, including OpenEXR's auxiliary codecs below, were
rechecked against source headers on 2026-11-02 for FreeImage 3.19.31. All eight
optional codec archives were verified against the SHA-256 digests in
`Source/ThirdParty/sources.json`; release/version files inside the archives
were also reviewed. Little CMS 2.20 reports `LCMS_VERSION` 2200 in its public
header and `AC_INIT(lcms2,2.20)` in its release configuration.

Bundled versions are unchanged since FreeImage 3.19.30.

Since 3.19.29, OpenEXR moved from 3.5.2 to 3.5.3, Little CMS from 2.19.1 to
2.20 (`LCMS_VERSION` 2200), and libaom from 3.15.2 to 3.15.3. Other bundled
versions are unchanged.
OpenEXR, LibDeflate and LibTIFF4 were previously re-synced for #137; OpenEXR
3.5.2 and libpng 1.6.59 followed in #148.

The scheduled checker now covers 20 directly vendored and archive-bundled
libraries, with separate tables for updates and current versions. OpenEXR's
Zstandard follows OpenEXR's releases; OpenJPH is updated separately below; LibJXR remains an
unversioned snapshot without a maintained upstream release feed. The
latest-upstream column above retains the previous release review; the
version/hash audit does not assert that every bundled dependency is the
latest available upstream release.

OpenEXR 3.5.3 also brings these upstream-vendored auxiliary codecs:

| Library | Path | Vendored | Version source |
|---|---|---|---|
| Zstandard | `Source/OpenEXR/Zstd` | 1.5.7 | `lib/zstd.h` |
| OpenJPH | `Source/OpenEXR/OpenJPH` | 0.33.0 | `openjph/ojph_version.h` |

Zstandard follows OpenEXR's release. OpenJPH was updated independently to
0.33.0 on 2026-11-03 for #259. That release fixes the HTJ2K one-row codeblock
heap overflow (CVE-2026-111151) and the NLT marker over-read. Its source file
lists are unchanged; FreeImage's x86 dispatch opt-in and ARM64EC guards are
retained.
See [vendor integration notes](scripts/vendor/NOTES.md) for source provenance,
configuration choices, and local patches to preserve in the next update.

Optional modern codecs (enabled with BUILD_LIBHEIF / BUILD_LIBJXL) now build
pinned source archives by default. USE_SYSTEM_LIBHEIF / USE_SYSTEM_LIBJXL
retain installed-SDK discovery. These archives and license copies are under
`Source/ThirdParty/`; SHA-256 digests and provenance are in `sources.json`.

| Library | Bundled | Purpose |
| --- | --- | --- |
| libheif | 1.23.7 | HEIF/AVIF container |
| libde265 | 1.1.3 | HEVC decoding |
| Kvazaar | 2.3.2 | HEVC encoding |
| libaom | 3.15.3 | AV1 decoding/encoding |
| libjxl | 0.12.0 | JPEG XL |
| Highway | 1.4.0 | JPEG XL SIMD |
| Brotli | 1.2.1 | JPEG XL compression |
| Little CMS | 2.20 | JPEG XL color management |

FreeImage's HEIF/AVIF plugin adapts FreeImageRe's FIPL/GPLv2-licensed decoder
at `b51d3945bfe785ad382a0969f1376aee344af8c6`; source archive bundling does
not change the separately licensed codec/backends. See
[licenses and update process](Source/ThirdParty/README.md).
