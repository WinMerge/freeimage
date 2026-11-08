# SourceForge SVN patch submissions

Last checked: **2026-11-05**, using the public SourceForge REST API.

The [local patch archive and current candidate table](upstream-patches/README.md)
contains all **29 original attachments** from our #115 and #170–197
submissions, ticket metadata, SHA-256 checksums, and patches exported from
committed fork changes that need submission review. Tickets #96 and #198
have no uploaded attachments; exports of their linked commits are provided
separately and marked as local supplements.

This index links the FreeImage fork's recorded submissions to the original
FreeImage project on SourceForge. The patch tracker is the submission venue;
the SVN repository is where an accepted change would land.

- [FreeImage patch tracker](https://sourceforge.net/p/freeimage/patches/)
- [FreeImage SVN code browser](https://sourceforge.net/p/freeimage/svn/)
- [Live patch index, page 1](https://sourceforge.net/rest/p/freeimage/patches?limit=100&page=0)
- [Live patch index, page 2](https://sourceforge.net/rest/p/freeimage/patches?limit=100&page=1)

There are **28 recent submissions with patch attachments (#170–197)**,
including the superseded #183 archive. A further submission ticket, #198,
links to fixes but currently has no patch attachment. Historical #115 adds
one more attached submission; #96 links to its original commit. The complete tracker
contains **197 tickets**, all linked in the appendix below.

The attachment links below were read from the live ticket API. All tickets
#170–198 returned an unspecified status. Submission or an attached patch does
not establish that a change was applied to SVN; no SVN acceptance revision
has been verified for this index. Tickets #170–173 list their reporter as
anonymous; the others in this recent series list `danoli3`.

SVN trunk still reports r1911, last changed on 2024-04-18. The fork review
baseline for the current candidate inventory is `origin/master` at
`f376729ec99dcb00f93d32e3ee26aaffdc55f4d8` (FreeImage 3.19.31).

## Historical submissions

| Ticket | What it changes | Local artifact | Tracker state |
| --- | --- | --- | --- |
| [#96](https://sourceforge.net/p/freeimage/patches/96/) | iOS/ARM64 build fixes for the older FreeImage tree | [Linked commit export](upstream-patches/sourceforge/96/linked-commits/manifest.json); no uploaded attachment | Closed; SVN acceptance revision not established; updated by #115 |
| [#115](https://sourceforge.net/p/freeimage/patches/115/) | FreeImage 3.17.0 iOS/macOS, arm64/x86_64, tvOS, build lists and vendor compatibility fixes | [FreeImage3.17.0.patch](upstream-patches/sourceforge/115/FreeImage3.17.0.patch) | Open; historical patch |

## Security and dependency submissions (#170–183)

| Ticket | Submitted change | Patch download | Fork reference | Local copy |
| --- | --- | --- | --- | --- |
| [#170](https://sourceforge.net/p/freeimage/patches/170/) | CVE-2020-24292: reject illegal ICO BITMAPINFOHEADER bpp/size | [CVE-2020-24292.patch](https://sourceforge.net/p/freeimage/patches/170/attachment/CVE-2020-24292.patch) | [pull 108](https://github.com/danoli3/FreeImage/pull/108) | [file](upstream-patches/sourceforge/170/CVE-2020-24292.patch) |
| [#171](https://sourceforge.net/p/freeimage/patches/171/) | CVE-2020-24293: bound psdThumbnail::Read packed-row copy | [CVE-2020-24293.patch](https://sourceforge.net/p/freeimage/patches/171/attachment/CVE-2020-24293.patch) | [pull 109](https://github.com/danoli3/FreeImage/pull/109) | [file](upstream-patches/sourceforge/171/CVE-2020-24293.patch) |
| [#172](https://sourceforge.net/p/freeimage/patches/172/) | CVE-2020-24295: bound psdParser::ReadImageLine destination writes | [CVE-2020-24295.patch](https://sourceforge.net/p/freeimage/patches/172/attachment/CVE-2020-24295.patch) | [pull 110](https://github.com/danoli3/FreeImage/pull/110) | [file](upstream-patches/sourceforge/172/CVE-2020-24295.patch) |
| [#173](https://sourceforge.net/p/freeimage/patches/173/) | CVE-2020-22524: fix PFM integer overflow in line buffer allocation | [CVE-2020-22524.patch](https://sourceforge.net/p/freeimage/patches/173/attachment/CVE-2020-22524.patch) | [commit 668385e5](https://github.com/danoli3/FreeImage/commit/668385e561a4a338f5b3513a6f09a0502933281e), [pull 56](https://github.com/danoli3/FreeImage/pull/56) | [file](upstream-patches/sourceforge/173/CVE-2020-22524.patch) |
| [#174](https://sourceforge.net/p/freeimage/patches/174/) | CVE-2020-24294 / CVE-2024-28565: clamp psdParser::UnpackRLE src and dst | [CVE-2020-24294.patch](https://sourceforge.net/p/freeimage/patches/174/attachment/CVE-2020-24294.patch) | [commit 62971de](https://github.com/danoli3/FreeImage/commit/62971de), [pull 54](https://github.com/danoli3/FreeImage/pull/54) | [file](upstream-patches/sourceforge/174/CVE-2020-24294.patch) |
| [#175](https://sourceforge.net/p/freeimage/patches/175/) | CVE-2024-28579 / CVE-2024-28582: reject overflowing HDR RLE scanline counts | [CVE-2024-28579.patch](https://sourceforge.net/p/freeimage/patches/175/attachment/CVE-2024-28579.patch) | [commit d866b72](https://github.com/danoli3/FreeImage/commit/d866b72), [pull 55](https://github.com/danoli3/FreeImage/pull/55) | [file](upstream-patches/sourceforge/175/CVE-2024-28579.patch) |
| [#176](https://sourceforge.net/p/freeimage/patches/176/) | CVE-2024-28584: NULL-check JPEG2000 image after opj_read_header | [CVE-2024-28584.patch](https://sourceforge.net/p/freeimage/patches/176/attachment/CVE-2024-28584.patch) | [commit 9286f7f](https://github.com/danoli3/FreeImage/commit/9286f7f), [pull 53](https://github.com/danoli3/FreeImage/pull/53) | [file](upstream-patches/sourceforge/176/CVE-2024-28584.patch) |
| [#177](https://sourceforge.net/p/freeimage/patches/177/) | CVE-2024-28581: TARGA RLE IOCache at least one pixel wide | [CVE-2024-28581.patch](https://sourceforge.net/p/freeimage/patches/177/attachment/CVE-2024-28581.patch) | [commit 68af81b](https://github.com/danoli3/FreeImage/commit/68af81b), [pull 39](https://github.com/danoli3/FreeImage/pull/39) | [file](upstream-patches/sourceforge/177/CVE-2024-28581.patch) |
| [#178](https://sourceforge.net/p/freeimage/patches/178/) | CVE-2024-28567: re-fetch TIFF ICC pointer after ExifIFD switch | [CVE-2024-28567.patch](https://sourceforge.net/p/freeimage/patches/178/attachment/CVE-2024-28567.patch) | [commit ce6fe58](https://github.com/danoli3/FreeImage/commit/ce6fe58), [pull 79](https://github.com/danoli3/FreeImage/pull/79) | [file](upstream-patches/sourceforge/178/CVE-2024-28567.patch) |
| [#179](https://sourceforge.net/p/freeimage/patches/179/) | CVE-2021-33367: bound Exif 0th-IFD next-offset read | [CVE-2021-33367.patch](https://sourceforge.net/p/freeimage/patches/179/attachment/CVE-2021-33367.patch) | [commit 5e1e391](https://github.com/danoli3/FreeImage/commit/5e1e391) | [file](upstream-patches/sourceforge/179/CVE-2021-33367.patch) |
| [#180](https://sourceforge.net/p/freeimage/patches/180/) | CVE-2024-28568/9029/28570/28573/28577/28578/28580: IPTC/Exif/XPM/RAS OOB | [CVE-2024-28568.patch](https://sourceforge.net/p/freeimage/patches/180/attachment/CVE-2024-28568.patch) | [commit d474400](https://github.com/danoli3/FreeImage/commit/d474400), [pull 96](https://github.com/danoli3/FreeImage/pull/96) | [file](upstream-patches/sourceforge/180/CVE-2024-28568.patch) |
| [#181](https://sourceforge.net/p/freeimage/patches/181/) | CVE-2024-28566/28571/28572/28583: TIFF AssignPixel, JPEG src, MakerNote, XBM | [CVE-2024-28566.patch](https://sourceforge.net/p/freeimage/patches/181/attachment/CVE-2024-28566.patch) | [commit 8ebe8e5](https://github.com/danoli3/FreeImage/commit/8ebe8e5), [pull 99](https://github.com/danoli3/FreeImage/pull/99) | [file](upstream-patches/sourceforge/181/CVE-2024-28566.patch) |
| [#182](https://sourceforge.net/p/freeimage/patches/182/) | CVE-2024-28574/28575/28576: replace bundled OpenJPEG 2.0.0 with 2.5.4 | [CVE-2024-28574.patch.gz](https://sourceforge.net/p/freeimage/patches/182/attachment/CVE-2024-28574.patch.gz) | [commit 229de19](https://github.com/danoli3/FreeImage/commit/229de19), [pull 68](https://github.com/danoli3/FreeImage/pull/68) | [file](upstream-patches/sourceforge/182/CVE-2024-28574.patch.gz) |
| [#183](https://sourceforge.net/p/freeimage/patches/183/) | CVE-2024-28562/28563/28564/28569: replace bundled OpenEXR 2.2 with 3.3.13 **Superseded by #197.** | [CVE-2024-28562.patch.gz](https://sourceforge.net/p/freeimage/patches/183/attachment/CVE-2024-28562.patch.gz) | [commit 248c6c9](https://github.com/danoli3/FreeImage/commit/248c6c9), [pull 69](https://github.com/danoli3/FreeImage/pull/69) | [file](upstream-patches/sourceforge/183/CVE-2024-28562.patch.gz) |

## Loader and pixel-conversion submissions (#184–189)

| Ticket | Submitted change | Patch download | Fork reference | Local copy |
| --- | --- | --- | --- | --- |
| [#184](https://sourceforge.net/p/freeimage/patches/184/) | Prevent scanline pointer underflow in CUT RLE decoding | [cut-rle-bounds-submission.zip](https://sourceforge.net/p/freeimage/patches/184/attachment/cut-rle-bounds-submission.zip) | [commit 9d62eb2](https://github.com/danoli3/FreeImage/commit/9d62eb2) | [file](upstream-patches/sourceforge/184/cut-rle-bounds-submission.zip) |
| [#185](https://sourceforge.net/p/freeimage/patches/185/) | Reject three-plane PCX images with insufficient bytes per plane | [pcx-plane-width-submission.zip](https://sourceforge.net/p/freeimage/patches/185/attachment/pcx-plane-width-submission.zip) | [commit a4473a4](https://github.com/danoli3/FreeImage/commit/a4473a4) | [file](upstream-patches/sourceforge/185/pcx-plane-width-submission.zip) |
| [#186](https://sourceforge.net/p/freeimage/patches/186/) | Preserve DDS DXT dimensions and decode trailing partial blocks | [dds-partial-blocks-submission.zip](https://sourceforge.net/p/freeimage/patches/186/attachment/dds-partial-blocks-submission.zip) | [commit bf820dc](https://github.com/danoli3/FreeImage/commit/bf820dc) | [file](upstream-patches/sourceforge/186/dds-partial-blocks-submission.zip) |
| [#187](https://sourceforge.net/p/freeimage/patches/187/) | Select RGB565 conversion using the distinguishing green mask | [rgb565-conversion-submission.zip](https://sourceforge.net/p/freeimage/patches/187/attachment/rgb565-conversion-submission.zip) | [commit 1a9ccb5](https://github.com/danoli3/FreeImage/commit/1a9ccb5) | [file](upstream-patches/sourceforge/187/rgb565-conversion-submission.zip) |
| [#188](https://sourceforge.net/p/freeimage/patches/188/) | Accept BMP information-header sizes already handled by the loader | [bmp-extended-headers-submission.zip](https://sourceforge.net/p/freeimage/patches/188/attachment/bmp-extended-headers-submission.zip) | [commit 7646cfe](https://github.com/danoli3/FreeImage/commit/7646cfe) | [file](upstream-patches/sourceforge/188/bmp-extended-headers-submission.zip) |
| [#189](https://sourceforge.net/p/freeimage/patches/189/) | Accept trailing BMP RLE8 end-of-bitmap after the final end-of-line | [bmp-rle8-terminator-submission.zip](https://sourceforge.net/p/freeimage/patches/189/attachment/bmp-rle8-terminator-submission.zip) | [commit b9c6227](https://github.com/danoli3/FreeImage/commit/b9c6227) | [file](upstream-patches/sourceforge/189/bmp-rle8-terminator-submission.zip) |

## SVN vendor/build follow-ups (#190–197)

| Ticket | Submitted change | Patch download | Fork reference | Local copy |
| --- | --- | --- | --- | --- |
| [#190](https://sourceforge.net/p/freeimage/patches/190/) | Complete the OpenJPEG 2.5.4 Windows project source list (follow-up to #182) | [openjpeg-2.5.4-project.patch.gz](https://sourceforge.net/p/freeimage/patches/190/attachment/openjpeg-2.5.4-project.patch.gz) | See ticket | [file](upstream-patches/sourceforge/190/openjpeg-2.5.4-project.patch.gz) |
| [#191](https://sourceforge.net/p/freeimage/patches/191/) | Update bundled zlib from 1.2.13 to 1.3.2 | [zlib-1.3.2.patch.gz](https://sourceforge.net/p/freeimage/patches/191/attachment/zlib-1.3.2.patch.gz) | See ticket | [file](upstream-patches/sourceforge/191/zlib-1.3.2.patch.gz) |
| [#192](https://sourceforge.net/p/freeimage/patches/192/) | Update bundled libpng from 1.6.39 to 1.6.59 | [libpng-1.6.59.patch.gz](https://sourceforge.net/p/freeimage/patches/192/attachment/libpng-1.6.59.patch.gz) | See ticket | [file](upstream-patches/sourceforge/192/libpng-1.6.59.patch.gz) |
| [#193](https://sourceforge.net/p/freeimage/patches/193/) | Update bundled WebP from 1.2.1 to 1.6.0 | [webp-1.6.0.patch.gz](https://sourceforge.net/p/freeimage/patches/193/attachment/webp-1.6.0.patch.gz) | See ticket | [file](upstream-patches/sourceforge/193/webp-1.6.0.patch.gz) |
| [#194](https://sourceforge.net/p/freeimage/patches/194/) | Update bundled libtiff from 4.5.0 to 4.7.2 | [libtiff-4.7.2.patch.gz](https://sourceforge.net/p/freeimage/patches/194/attachment/libtiff-4.7.2.patch.gz) | See ticket | [file](upstream-patches/sourceforge/194/libtiff-4.7.2.patch.gz) |
| [#195](https://sourceforge.net/p/freeimage/patches/195/) | Update bundled LibRawLite from 0.21.1 to 0.22.2 | [libraw-0.22.2.patch.gz](https://sourceforge.net/p/freeimage/patches/195/attachment/libraw-0.22.2.patch.gz) | See ticket | [file](upstream-patches/sourceforge/195/libraw-0.22.2.patch.gz) |
| [#196](https://sourceforge.net/p/freeimage/patches/196/) | Update bundled IJG JPEG from 9d to 10.0 | [jpeg-10.0.patch.gz](https://sourceforge.net/p/freeimage/patches/196/attachment/jpeg-10.0.patch.gz) | See ticket | [file](upstream-patches/sourceforge/196/jpeg-10.0.patch.gz) |
| [#197](https://sourceforge.net/p/freeimage/patches/197/) | Replace #183 with OpenEXR 3.5.2, Imath 3.2.3 and libdeflate 1.26 | [openexr-3.5.2.patch.gz](https://sourceforge.net/p/freeimage/patches/197/attachment/openexr-3.5.2.patch.gz) | See ticket | [file](upstream-patches/sourceforge/197/openexr-3.5.2.patch.gz) |

## Patch dependencies and replacements

- #169 is an accidental empty ICO ticket, replaced by #170. It is listed in
  the full inventory for completeness, and excluded from the 28 attached submissions.
- Apply #179 before the later Exif changes, #180 before #181, and #178 before
  #181's TIFF changes, as recorded in the original submission series.
- The vendor series targets SVN trunk r1911. Apply #182 first, then #190,
  #191, #192, #193, #194, #195, #196 and #197 in order; shared build files
  make these dependent patches.
- **#197 replaces #183. Use #197 instead of #183; do not apply both.**
  #190 supplements #182 with its Windows project source list.
- For the vendor `.patch.gz` files, decompress and follow the ticket's
  `patch --dry-run -E -p0` / `patch -E -p0` instructions from the tree root.
  ZIP submissions include their own patch and validation notes.
- #190's original decompressed patch lacks a final newline and fails strict
  Git patch parsing. The original is retained unchanged; a separately labelled
  [local companion](upstream-patches/sourceforge/190/openjpeg-2.5.4-project-with-final-newline.patch)
  appends exactly one LF byte and passes parsing. It has not been uploaded.

## Follow-up ticket without a patch attachment

[#198: Harden RAS and IFF PBM RLE image decoders](https://sourceforge.net/p/freeimage/patches/198/)
was created on 2026-10-17 by `danoli3`. It links RAS dimension/allocation
hardening and IFF PBM RLE bounds, short-read and no-op handling to fork commits:

- [RAS fix](https://github.com/danoli3/FreeImage/commit/d474400d1434624fd3fc6f0503a6ac0a7fbc1b0b)
- [IFF bounds fix](https://github.com/danoli3/FreeImage/commit/38fbe82)
- [IFF short-read/no-op follow-up](https://github.com/danoli3/FreeImage/commit/ad068d2)

The live API returned no attachment for #198 on the check date,
so it is recorded as a ticket with source links rather than a downloadable patch.
The [local linked-commit exports](upstream-patches/sourceforge/198/linked-commits/manifest.json)
are supplements, not submitted attachments. The RAS change already overlaps #180.

## Changes needing submission review

The [current candidate table](upstream-patches/README.md#changes-not-found-in-our-submitted-patch-attachments)
includes local patches for later security fixes, core behavior, new APIs and
build changes, with original commit and parent references. High-priority
items include GIF frame bounds, TARGA RLE bounds, BMP RLE4 arithmetic and
cursor overflow, memory-stream byte counts/write bounds, oversized ICOs,
TIFF/JPEG sampling mismatch handling, and JP2/J2K callback validation.

These are committed fork diffs for review, not a series validated against
clean SVN. Related historical patches (including #92, #143, #149, #161 and
#163) must be compared before claiming a new submission is needed.

Downloaded vendor snapshots already contain Android OpenEXR allocation,
strict-C11 libdeflate Linux ARM declarations, and WebP/libdeflate ARM64EC
guards (#193/#197). Those fixes should not be submitted again separately.
The [complete fork commit inventory](upstream-patches/FORK_COMMIT_INVENTORY.md)
also records changes outside the focused candidate selection.

## Complete SourceForge patch tracker index

All **197 tickets** returned by the two live index pages, newest number first.
This includes other contributors' submissions, closed historical tickets,
superseded submissions and empty stubs. A row establishes that the ticket
exists; attachment contents and SVN application were not audited for the
historical entries in this appendix.

| Ticket | Submitted subject |
| --- | --- |
| [#198](https://sourceforge.net/p/freeimage/patches/198/) | Harden RAS and IFF PBM RLE image decoders |
| [#197](https://sourceforge.net/p/freeimage/patches/197/) | Replace #183 with OpenEXR 3.5.2, Imath 3.2.3 and libdeflate 1.26 |
| [#196](https://sourceforge.net/p/freeimage/patches/196/) | Update bundled IJG JPEG from 9d to 10.0 |
| [#195](https://sourceforge.net/p/freeimage/patches/195/) | Update bundled LibRawLite from 0.21.1 to 0.22.2 |
| [#194](https://sourceforge.net/p/freeimage/patches/194/) | Update bundled libtiff from 4.5.0 to 4.7.2 |
| [#193](https://sourceforge.net/p/freeimage/patches/193/) | Update bundled WebP from 1.2.1 to 1.6.0 |
| [#192](https://sourceforge.net/p/freeimage/patches/192/) | Update bundled libpng from 1.6.39 to 1.6.59 |
| [#191](https://sourceforge.net/p/freeimage/patches/191/) | Update bundled zlib from 1.2.13 to 1.3.2 |
| [#190](https://sourceforge.net/p/freeimage/patches/190/) | Complete the OpenJPEG 2.5.4 Windows project source list (follow-up to #182) |
| [#189](https://sourceforge.net/p/freeimage/patches/189/) | Accept trailing BMP RLE8 end-of-bitmap after the final end-of-line |
| [#188](https://sourceforge.net/p/freeimage/patches/188/) | Accept BMP information-header sizes already handled by the loader |
| [#187](https://sourceforge.net/p/freeimage/patches/187/) | Select RGB565 conversion using the distinguishing green mask |
| [#186](https://sourceforge.net/p/freeimage/patches/186/) | Preserve DDS DXT dimensions and decode trailing partial blocks |
| [#185](https://sourceforge.net/p/freeimage/patches/185/) | Reject three-plane PCX images with insufficient bytes per plane |
| [#184](https://sourceforge.net/p/freeimage/patches/184/) | Prevent scanline pointer underflow in CUT RLE decoding |
| [#183](https://sourceforge.net/p/freeimage/patches/183/) | CVE-2024-28562/28563/28564/28569: replace bundled OpenEXR 2.2 with 3.3.13 |
| [#182](https://sourceforge.net/p/freeimage/patches/182/) | CVE-2024-28574/28575/28576: replace bundled OpenJPEG 2.0.0 with 2.5.4 |
| [#181](https://sourceforge.net/p/freeimage/patches/181/) | CVE-2024-28566/28571/28572/28583: TIFF AssignPixel, JPEG src, MakerNote, XBM |
| [#180](https://sourceforge.net/p/freeimage/patches/180/) | CVE-2024-28568/9029/28570/28573/28577/28578/28580: IPTC/Exif/XPM/RAS OOB |
| [#179](https://sourceforge.net/p/freeimage/patches/179/) | CVE-2021-33367: bound Exif 0th-IFD next-offset read |
| [#178](https://sourceforge.net/p/freeimage/patches/178/) | CVE-2024-28567: re-fetch TIFF ICC pointer after ExifIFD switch |
| [#177](https://sourceforge.net/p/freeimage/patches/177/) | CVE-2024-28581: TARGA RLE IOCache at least one pixel wide |
| [#176](https://sourceforge.net/p/freeimage/patches/176/) | CVE-2024-28584: NULL-check JPEG2000 image after opj_read_header |
| [#175](https://sourceforge.net/p/freeimage/patches/175/) | CVE-2024-28579 / CVE-2024-28582: reject overflowing HDR RLE scanline counts |
| [#174](https://sourceforge.net/p/freeimage/patches/174/) | CVE-2020-24294 / CVE-2024-28565: clamp psdParser::UnpackRLE src and dst |
| [#173](https://sourceforge.net/p/freeimage/patches/173/) | CVE-2020-22524: fix PFM integer overflow in line buffer allocation |
| [#172](https://sourceforge.net/p/freeimage/patches/172/) | CVE-2020-24295: bound psdParser::ReadImageLine destination writes |
| [#171](https://sourceforge.net/p/freeimage/patches/171/) | CVE-2020-24293: bound psdThumbnail::Read packed-row copy |
| [#170](https://sourceforge.net/p/freeimage/patches/170/) | CVE-2020-24292: reject illegal ICO BITMAPINFOHEADER bpp/size |
| [#169](https://sourceforge.net/p/freeimage/patches/169/) | CVE-2020-24292: reject illegal ICO BITMAPINFOHEADER bpp/size |
| [#168](https://sourceforge.net/p/freeimage/patches/168/) | Any chance for a WinARM64 build of FreeImage.dll? |
| [#167](https://sourceforge.net/p/freeimage/patches/167/) | FreeImage 3.18.0 Canon MakerNote out-of-bounds read |
| [#166](https://sourceforge.net/p/freeimage/patches/166/) | FreeImage_Save fails to save in DDS file format |
| [#165](https://sourceforge.net/p/freeimage/patches/165/) | Fix for #362 (CVE-2023-47997) |
| [#164](https://sourceforge.net/p/freeimage/patches/164/) | Fix for #360 (CVE-2023-47995) |
| [#163](https://sourceforge.net/p/freeimage/patches/163/) | Add palette pointer check in PluginRAS.cpp |
| [#162](https://sourceforge.net/p/freeimage/patches/162/) | Add check for psd file channels. |
| [#161](https://sourceforge.net/p/freeimage/patches/161/) | Two patch fix out of bounds. |
| [#160](https://sourceforge.net/p/freeimage/patches/160/) | Limit input psd image color depth. |
| [#159](https://sourceforge.net/p/freeimage/patches/159/) | psdParser::UnpackRLE() memory out of bounds. |
| [#158](https://sourceforge.net/p/freeimage/patches/158/) | Numeric Trunctation at `misc_parsers.cpp:257` |
| [#157](https://sourceforge.net/p/freeimage/patches/157/) | Fix integer wrap around and segmentation fault in Exif.cpp |
| [#156](https://sourceforge.net/p/freeimage/patches/156/) | Fixed some issues with the EXR file reading |
| [#155](https://sourceforge.net/p/freeimage/patches/155/) | Bump LibRaw version |
| [#154](https://sourceforge.net/p/freeimage/patches/154/) | Fix out of bounds read in PluginBMP |
| [#153](https://sourceforge.net/p/freeimage/patches/153/) | patch for building 3.18.0 on big-endian hosts |
| [#152](https://sourceforge.net/p/freeimage/patches/152/) | Bugfix for #346 - Fix heap corruption when reading TIFFTAG_RICHTIFFIPTC |
| [#151](https://sourceforge.net/p/freeimage/patches/151/) | Updates OpenEXR to version 3.1.3 |
| [#150](https://sourceforge.net/p/freeimage/patches/150/) | Avoids clamping to [0..1] when converting bitmaps from one Float format to another |
| [#149](https://sourceforge.net/p/freeimage/patches/149/) | Fix 5 crashes revealed by oss-fuzz |
| [#148](https://sourceforge.net/p/freeimage/patches/148/) | Bugfix for #340 (Calling FreeImage_Paste fails on very large images) |
| [#147](https://sourceforge.net/p/freeimage/patches/147/) | Bugfix for #339 |
| [#146](https://sourceforge.net/p/freeimage/patches/146/) | RAII for easier memory management |
| [#145](https://sourceforge.net/p/freeimage/patches/145/) | Build patches used in homebrew |
| [#144](https://sourceforge.net/p/freeimage/patches/144/) | E2K: fixed build by MCST lcc compiler (MCST Elbrus 2000 architecture) |
| [#143](https://sourceforge.net/p/freeimage/patches/143/) | Bugfix for #299,#303,#304,#305,#308,#309,#310, #320  |
| [#142](https://sourceforge.net/p/freeimage/patches/142/) | Bugfix for bugs #306 #307 as well as problem from patch #139 |
| [#141](https://sourceforge.net/p/freeimage/patches/141/) | CMake support patch |
| [#140](https://sourceforge.net/p/freeimage/patches/140/) | Few patches from vspkg |
| [#139](https://sourceforge.net/p/freeimage/patches/139/) | Solve the problem of PluginTARGA |
| [#138](https://sourceforge.net/p/freeimage/patches/138/) | TIFF Deflate compression fixes |
| [#137](https://sourceforge.net/p/freeimage/patches/137/) | New format AVIF (AV1 Image File) |
| [#136](https://sourceforge.net/p/freeimage/patches/136/) | Implement TFreeWinBitmap.Draw with a custom source rectangle. |
| [#135](https://sourceforge.net/p/freeimage/patches/135/) | fix big endian builds |
| [#134](https://sourceforge.net/p/freeimage/patches/134/) | FreeImage_LoadFromMemory is unnecessarily slow |
| [#133](https://sourceforge.net/p/freeimage/patches/133/) | Need an extra EXIF tag for TIFF |
| [#132](https://sourceforge.net/p/freeimage/patches/132/) | Additional RAW metadata |
| [#131](https://sourceforge.net/p/freeimage/patches/131/) | Tiff embedded icc profiles are overwritten before use |
| [#130](https://sourceforge.net/p/freeimage/patches/130/) | freeimage fails to build against jpeg-9c |
| [#129](https://sourceforge.net/p/freeimage/patches/129/) | Error-checking is lacking |
| [#128](https://sourceforge.net/p/freeimage/patches/128/) | Add const-poisoning |
| [#127](https://sourceforge.net/p/freeimage/patches/127/) | Fixed strange error handling in PNG plugin |
| [#126](https://sourceforge.net/p/freeimage/patches/126/) | Visual Studio 2015 project files |
| [#125](https://sourceforge.net/p/freeimage/patches/125/) | Additional FIF_RAW Flags |
| [#124](https://sourceforge.net/p/freeimage/patches/124/) | Delphi 10.1 Berlin support for wrapper |
| [#123](https://sourceforge.net/p/freeimage/patches/123/) | Bugfix: Tiff thumbnails fails to load if exif is present |
| [#122](https://sourceforge.net/p/freeimage/patches/122/) | Refactor: Streamline MultiPage and CacheFile |
| [#121](https://sourceforge.net/p/freeimage/patches/121/) | PluginDDS Bug Fix |
| [#120](https://sourceforge.net/p/freeimage/patches/120/) | Bugfix: Image View broken save as BMP |
| [#119](https://sourceforge.net/p/freeimage/patches/119/) | Bugfix: Cloned bitmap "view" still points to original external bitmap |
| [#118](https://sourceforge.net/p/freeimage/patches/118/) | Bugfix: ColorType is incorrect for 16bit CMYK |
| [#117](https://sourceforge.net/p/freeimage/patches/117/) | Bugfix: Color Profile is actually still there |
| [#116](https://sourceforge.net/p/freeimage/patches/116/) | Jpeg CMYK Save support |
| [#115](https://sourceforge.net/p/freeimage/patches/115/) | Fixes for FreeImage - iOS, OSX, arm64 and x86_64 |
| [#114](https://sourceforge.net/p/freeimage/patches/114/) | OS X 10.11.3, make is not working. |
| [#113](https://sourceforge.net/p/freeimage/patches/113/) | Delphi Wrapper updated for 3.18.0 / Seattle |
| [#112](https://sourceforge.net/p/freeimage/patches/112/) | Wrong colors in PNG image |
| [#111](https://sourceforge.net/p/freeimage/patches/111/) | PSD write support |
| [#110](https://sourceforge.net/p/freeimage/patches/110/) | fixes for GCC 5.0 |
| [#109](https://sourceforge.net/p/freeimage/patches/109/) | Delphi wrapper 3.17.0 update |
| [#108](https://sourceforge.net/p/freeimage/patches/108/) | Visual Studio 2015 supports snprintf |
| [#107](https://sourceforge.net/p/freeimage/patches/107/) | FreeImage_ConvertToRawBits support for transparency from palletized to 32bit |
| [#106](https://sourceforge.net/p/freeimage/patches/106/) | Compile FreeImage on Mac OSX 10.10 Yosemite |
| [#105](https://sourceforge.net/p/freeimage/patches/105/) | Minor bug in several Makefiles  |
| [#104](https://sourceforge.net/p/freeimage/patches/104/) | New function: FreeImage_RescaleEx |
| [#103](https://sourceforge.net/p/freeimage/patches/103/) | Fixed PNG plugin handling of 24-bit RGB + tRNS chunk images |
| [#102](https://sourceforge.net/p/freeimage/patches/102/) | Fixed PNG plugin handling of 16-bit grayscale + 16-bit alpha images |
| [#101](https://sourceforge.net/p/freeimage/patches/101/) | New function: FreeImage_ConvertToRGBAF |
| [#100](https://sourceforge.net/p/freeimage/patches/100/) | New function: FreeImage_GetMemorySize |
| [#99](https://sourceforge.net/p/freeimage/patches/99/) | New Lossless Fast Pseudo-Quantization algorithm |
| [#98](https://sourceforge.net/p/freeimage/patches/98/) | New function: FreeImage_CreateView |
| [#97](https://sourceforge.net/p/freeimage/patches/97/) | Explicit definition of endianness and color order |
| [#96](https://sourceforge.net/p/freeimage/patches/96/) | Fixes for FreeImage - iOS and arm64 |
| [#95](https://sourceforge.net/p/freeimage/patches/95/) | Infinite Loop Determining File Type for Truncated ANI (RIFF) |
| [#93](https://sourceforge.net/p/freeimage/patches/93/) | Segfault on Corrupt Animated GIF |
| [#92](https://sourceforge.net/p/freeimage/patches/92/) | Increase Speed of GIF_PLAYBACK Successive Frames |
| [#91](https://sourceforge.net/p/freeimage/patches/91/) | Delphi wrapper version 3.16.1 |
| [#90](https://sourceforge.net/p/freeimage/patches/90/) | Patch overflow bug on CalculateScanline |
| [#89](https://sourceforge.net/p/freeimage/patches/89/) | FreeImage CompileTime Configuration Support |
| [#88](https://sourceforge.net/p/freeimage/patches/88/) | Massively improved JPEG Transform |
| [#87](https://sourceforge.net/p/freeimage/patches/87/) | NetBSD and memalign |
| [#86](https://sourceforge.net/p/freeimage/patches/86/) | Create image from user bits without copy of pixel data |
| [#85](https://sourceforge.net/p/freeimage/patches/85/) | Minor issues in Resize resulting in lost performance |
| [#84](https://sourceforge.net/p/freeimage/patches/84/) | Recognize rgb as valid SGI files |
| [#83](https://sourceforge.net/p/freeimage/patches/83/) | Delphi wrapper update |
| [#82](https://sourceforge.net/p/freeimage/patches/82/) | TGA file read failure on valid TGA file |
| [#81](https://sourceforge.net/p/freeimage/patches/81/) | include memset.h for ImfAutoArray.h for G++ |
| [#80](https://sourceforge.net/p/freeimage/patches/80/) | include string.h for memset |
| [#79](https://sourceforge.net/p/freeimage/patches/79/) | Visual Studio 2012 Solution supporting batch builds |
| [#78](https://sourceforge.net/p/freeimage/patches/78/) | Suggested changes for Plugin.cpp |
| [#77](https://sourceforge.net/p/freeimage/patches/77/) | delphi wrapper 3.15.3 |
| [#76](https://sourceforge.net/p/freeimage/patches/76/) | Override builtin Plugins with a Local (or external) one |
| [#75](https://sourceforge.net/p/freeimage/patches/75/) | add missing headers for mingw |
| [#74](https://sourceforge.net/p/freeimage/patches/74/) | Use correct mime for file types |
| [#73](https://sourceforge.net/p/freeimage/patches/73/) | EXIF_MAIN write support for PluginTIFF |
| [#72](https://sourceforge.net/p/freeimage/patches/72/) | io.h to asm/io.h |
| [#71](https://sourceforge.net/p/freeimage/patches/71/) | libjpeg-turbo for Linux |
| [#70](https://sourceforge.net/p/freeimage/patches/70/) | tiff without samplesperpixel or bitspersample |
| [#69](https://sourceforge.net/p/freeimage/patches/69/) | Update Delphi Wrapper for 64 bit |
| [#68](https://sourceforge.net/p/freeimage/patches/68/) | Add half size flag to raw decoding |
| [#67](https://sourceforge.net/p/freeimage/patches/67/) | update Delphi wrapper for 3.15.0 |
| [#66](https://sourceforge.net/p/freeimage/patches/66/) | little update for delphi examples |
| [#65](https://sourceforge.net/p/freeimage/patches/65/) | Callbacks support |
| [#64](https://sourceforge.net/p/freeimage/patches/64/) | delphi wrapper update 3.14.1 |
| [#63](https://sourceforge.net/p/freeimage/patches/63/) | fix missing header |
| [#62](https://sourceforge.net/p/freeimage/patches/62/) | Read 16 bit BMPs Correctly |
| [#61](https://sourceforge.net/p/freeimage/patches/61/) | 32 and 24 bit conversion enhancements |
| [#60](https://sourceforge.net/p/freeimage/patches/60/) | Fix leaks in FreeImage_OpenMultiBitmapFromHandle() |
| [#59](https://sourceforge.net/p/freeimage/patches/59/) | Several bugs in FreeImage_CloseMultiBitmap() |
| [#58](https://sourceforge.net/p/freeimage/patches/58/) | Fix lots of leaks in FreeImage_OpenMultiBitmap() |
| [#57](https://sourceforge.net/p/freeimage/patches/57/) | FreeImage_GetOriginalBPP |
| [#56](https://sourceforge.net/p/freeimage/patches/56/) | Thumbnail support |
| [#55](https://sourceforge.net/p/freeimage/patches/55/) | delphi wrapper update |
| [#54](https://sourceforge.net/p/freeimage/patches/54/) | FreeImage_CloseMultiBitmap() without saving to file |
| [#53](https://sourceforge.net/p/freeimage/patches/53/) | Fix TIFFSetField call for TIFFTAG_PAGENUMBER |
| [#52](https://sourceforge.net/p/freeimage/patches/52/) | Extended Tiff Plugin |
| [#51](https://sourceforge.net/p/freeimage/patches/51/) | VS2005 x64 sln and prj update |
| [#50](https://sourceforge.net/p/freeimage/patches/50/) | Refactored PSD image data loading |
| [#49](https://sourceforge.net/p/freeimage/patches/49/) | FreeImage_CloseMultiBitmap() without saving to file |
| [#48](https://sourceforge.net/p/freeimage/patches/48/) | Support for additional channels (beyond RGBA) |
| [#47](https://sourceforge.net/p/freeimage/patches/47/) | Fix for unnecessary EXR loading limitation |
| [#46](https://sourceforge.net/p/freeimage/patches/46/) | Updated Example for Linux |
| [#45](https://sourceforge.net/p/freeimage/patches/45/) | FreeImage_ConvertTo32Bits performance hit |
| [#44](https://sourceforge.net/p/freeimage/patches/44/) | fix for PluginGIF saving corrupeted image (test image) |
| [#43](https://sourceforge.net/p/freeimage/patches/43/) | JPEG plugin standard optimized encoding support |
| [#42](https://sourceforge.net/p/freeimage/patches/42/) | JPG plugin comment storing improvements |
| [#41](https://sourceforge.net/p/freeimage/patches/41/) | fix for PluginGIF saving corrupeted image |
| [#40](https://sourceforge.net/p/freeimage/patches/40/) | Refactoring TARGA plugin (cont)  |
| [#39](https://sourceforge.net/p/freeimage/patches/39/) | Don't use libmng_data.h |
| [#38](https://sourceforge.net/p/freeimage/patches/38/) | Quickie to extract first frame of MNG |
| [#37](https://sourceforge.net/p/freeimage/patches/37/) | [TGA]Corrupt rle count fix |
| [#36](https://sourceforge.net/p/freeimage/patches/36/) | Quickie to extract first frame of MNG |
| [#35](https://sourceforge.net/p/freeimage/patches/35/) | Quickie to extract first frame of MNG |
| [#34](https://sourceforge.net/p/freeimage/patches/34/) | Quickie to extract first frame of MNG |
| [#33](https://sourceforge.net/p/freeimage/patches/33/) | Quickie to extract first frame of MNG |
| [#32](https://sourceforge.net/p/freeimage/patches/32/) | Crash on corrupted tags |
| [#31](https://sourceforge.net/p/freeimage/patches/31/) | Addition of FreeImage_LoadHeader function group |
| [#30](https://sourceforge.net/p/freeimage/patches/30/) | VS 2008 and X64 Windows builds |
| [#29](https://sourceforge.net/p/freeimage/patches/29/) | Background filling routines patch |
| [#28](https://sourceforge.net/p/freeimage/patches/28/) | Fixed some typos in README.minGW |
| [#27](https://sourceforge.net/p/freeimage/patches/27/) | Fixed some typos in README.minGW |
| [#26](https://sourceforge.net/p/freeimage/patches/26/) | Macro DEPRECATE fails on x64 |
| [#25](https://sourceforge.net/p/freeimage/patches/25/) | MinGW Makefile |
| [#24](https://sourceforge.net/p/freeimage/patches/24/) | New 'monochrome' function fixes ConvertToStandardType and... |
| [#23](https://sourceforge.net/p/freeimage/patches/23/) | Reinhard05 fix-up |
| [#22](https://sourceforge.net/p/freeimage/patches/22/) | Open multipage bitmap from handle |
| [#21](https://sourceforge.net/p/freeimage/patches/21/) | Refactoring TARGA plugin |
| [#20](https://sourceforge.net/p/freeimage/patches/20/) | [PATCH] JPEG rotate_exif bugs Version 3.12.0 |
| [#19](https://sourceforge.net/p/freeimage/patches/19/) | fix for PluginBMP alignment bug while saving 16 or 24bit BMP |
| [#18](https://sourceforge.net/p/freeimage/patches/18/) | Patch to allow x64 build |
| [#17](https://sourceforge.net/p/freeimage/patches/17/) | Upgrade to VS2008 |
| [#16](https://sourceforge.net/p/freeimage/patches/16/) | [PATCH]  Fix handling of frame disposal in GIF_PLAYBACK mode |
| [#15](https://sourceforge.net/p/freeimage/patches/15/) | Added support for SGI Grayscale + Alpha pics |
| [#14](https://sourceforge.net/p/freeimage/patches/14/) | added PNG Interlacing and compression support |
| [#13](https://sourceforge.net/p/freeimage/patches/13/) | Fix : SGI files with 2 dimesions crash |
| [#12](https://sourceforge.net/p/freeimage/patches/12/) | Fix and clean OSX makefile |
| [#11](https://sourceforge.net/p/freeimage/patches/11/) | VC8 compatibility latest CVS |
| [#10](https://sourceforge.net/p/freeimage/patches/10/) | linux fixes |
| [#9](https://sourceforge.net/p/freeimage/patches/9/) | Fix for Bug in PluginGIF.cpp |
| [#8](https://sourceforge.net/p/freeimage/patches/8/) | FreeImagePlus const correctness, fipMemoryIO copy bug |
| [#7](https://sourceforge.net/p/freeimage/patches/7/) | DDS patch for bad DDS files |
| [#6](https://sourceforge.net/p/freeimage/patches/6/) | SGI plugin |
| [#5](https://sourceforge.net/p/freeimage/patches/5/) | dds memory leak |
| [#4](https://sourceforge.net/p/freeimage/patches/4/) | DDS crash fix - FreeImag 3.8.0 |
| [#3](https://sourceforge.net/p/freeimage/patches/3/) | Cygwin Makefile |
| [#2](https://sourceforge.net/p/freeimage/patches/2/) | fipImage construction bug |
| [#1](https://sourceforge.net/p/freeimage/patches/1/) | FloydSteinberg memory leak |

## Maintaining this index

For each new submission, add its SourceForge ticket, attachment URL, fork
PR/commit and any prerequisite or replacement relationship. Keep tickets
without attachments distinct from downloadable patches. Record an SVN
revision and verify the corresponding source change before marking a patch
as applied upstream. Refresh the full inventory and check date from every
page of the live API, rather than relying on the default HTML tracker view.
