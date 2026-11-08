# Update bundled libpng from 1.6.39 to 1.6.59

Prepared 2026-10-13.

Replace Source/LibPNG with the fork-tested 1.6.59 sources, preserving the prebuilt pnglibconf configuration. Keep ARM NEON disabled as in the tested fork configuration; this patch does not add a new ARM optimization requirement.

## Patch series and application

Start from clean FreeImage SVN trunk r1911. Apply the existing OpenJPEG source upgrade from https://sourceforge.net/p/freeimage/patches/182/ first. Then apply this ordered vendor series:

1. zlib-1.3.2.patch.gz
2. libpng-1.6.59.patch.gz
3. webp-1.6.0.patch.gz
4. libtiff-4.7.2.patch.gz
5. libraw-0.22.2.patch.gz
6. jpeg-10.0.patch.gz
7. openexr-3.5.2.patch.gz (replacement follow-up #197 to #183)

These are separate reviewable patches with shared build-file prerequisites, not seven independent diffs against pristine SVN. The OpenEXR replacement must be used instead of the old #183 attachment: do not apply both. The OpenJPEG Windows-project companion is submitted as #190, supplementing #182; apply it after the existing #182 source patch.

For each patch, decompress it and apply from the FreeImage tree root with `patch --dry-run -E -p0`, then `patch -E -p0`. `-E` removes obsolete source/header files, including old Imath headers that must not shadow the new ones.

## Scope and verification

Each patch carries vendor code/configuration and reconciles GNU/macOS make source/header lists plus legacy VS source lists. File encodings and line endings are preserved. The library snapshots come from the current fork versions recorded at https://github.com/danoli3/FreeImage/blob/master/THIRD_PARTY_VERSIONS.md ; this is not a claim that every version is the newest available upstream release today. Retain all FreeImage and vendor license notices.

All patches dry-run and apply in order against r1911 plus the existing #182 source patch. The final combined tree builds a static library with GNU make and Apple Clang on macOS ARM64. An existing SVN jxrlib missing-declaration problem required a validation-only forced header declaring `_byteswap_ulong` and including wchar.h; that unrelated fix is not mixed into these vendor patches.

Runtime validation of the combined tree passed PNG, JPEG, lossless WebP and JPEG-compressed TIFF memory round trips; FreeImage zlib compression/decompression; EXR ZIP, PIZ, ZSTD, HTJ2K256, HTJ2K32, LJ2K and variable-sample deep ZSTD; TIFF uncompressed/LZW/Deflate/PackBits; and raw G3 fax decoding. The linked LibRaw version reports 0.22.2-Release. Real-camera RAW decoding was not tested in this batch. VS project XML and every referenced compilation source were checked, but these exact adapted SVN patches have not been compiled on Windows, Android, Linux or 32-bit runners. This combined validation is not a separate build of every intermediate series step or a full ASan run.

The original SVN FreeImage version remains 3.19.0; these are dependency updates, not an upstream release-number bump. Upstream acceptance still requires an SVN revision.
