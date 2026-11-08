# Upstream patch tracker

Current archive and submission review (**2026-11-05**):
[patch files and fix/status tables](upstream-patches/README.md),
[SourceForge submission index](SOURCEFORGE_SVN_PATCHES.md), and
[complete fork commit inventory](upstream-patches/FORK_COMMIT_INVENTORY.md).
The dated 2026-10-13 snapshot below is retained as preparation history;
use the current catalog for #96/#115/#198, later fork fixes and vendor overlap.

Last checked: 2026-10-13. Fork baseline: `origin/master` at `5af27d0` (3.19.21). This document distinguishes fixes merged here, submissions to the original FreeImage project, and acceptance into upstream SVN.

Sources: [SourceForge patches](https://sourceforge.net/p/freeimage/patches/), its [live REST index](https://sourceforge.net/rest/p/freeimage/patches?limit=100), [fork changelog](CHANGELOG.md), and linked commits. The live API listed 182 tickets before this batch and 196 after the six loader submissions and eight vendor follow-ups, across two pages. The cached HTML index missed recent submissions.

## Status rules

- **Merged here:** the linked fix is in the audited fork baseline.
- **Submitted:** a SourceForge ticket has a patch attachment and identifies our change. This does not mean upstream applied it.
- **Needs submission check:** merged here, but no matching ticket was identified in the index; inspect related tickets and comments before posting.
- **Needs audit:** a listed upstream patch has not been compared against our source and upstream SVN.
- **Applied upstream:** requires an upstream SVN revision and a source comparison. No entry below has been verified to meet this condition yet.

SourceForge returned an empty status field for tickets #169–183. Record that as unspecified, not accepted, closed, or rejected. Do not infer acceptance from an attachment or a related SVN link.

## Fixes and vendor updates with verified submissions

Each ticket below has a verified patch attachment and linked source evidence. Vendor follow-ups include adaptations for SVN build files; upstream application remains unverified.

| Upstream ticket | Fix | Fork evidence | Submission / acceptance |
| --- | --- | --- | --- |
| [#170](https://sourceforge.net/p/freeimage/patches/170/) | CVE-2020-24292: reject illegal ICO BITMAPINFOHEADER bpp/size | [pull 108](https://github.com/danoli3/FreeImage/pull/108) | Submitted; status unspecified; SVN application unverified |
| [#171](https://sourceforge.net/p/freeimage/patches/171/) | CVE-2020-24293: bound psdThumbnail::Read packed-row copy | [pull 109](https://github.com/danoli3/FreeImage/pull/109) | Submitted; status unspecified; SVN application unverified |
| [#172](https://sourceforge.net/p/freeimage/patches/172/) | CVE-2020-24295: bound psdParser::ReadImageLine destination writes | [pull 110](https://github.com/danoli3/FreeImage/pull/110) | Submitted; status unspecified; SVN application unverified |
| [#173](https://sourceforge.net/p/freeimage/patches/173/) | CVE-2020-22524: fix PFM integer overflow in line buffer allocation | [commit 668385e5](https://github.com/danoli3/FreeImage/commit/668385e561a4a338f5b3513a6f09a0502933281e), [pull 56](https://github.com/danoli3/FreeImage/pull/56) | Submitted; status unspecified; SVN application unverified |
| [#174](https://sourceforge.net/p/freeimage/patches/174/) | CVE-2020-24294 / CVE-2024-28565: clamp psdParser::UnpackRLE src and dst | [commit 62971de](https://github.com/danoli3/FreeImage/commit/62971de), [pull 54](https://github.com/danoli3/FreeImage/pull/54) | Submitted; status unspecified; SVN application unverified |
| [#175](https://sourceforge.net/p/freeimage/patches/175/) | CVE-2024-28579 / CVE-2024-28582: reject overflowing HDR RLE scanline counts | [commit d866b72](https://github.com/danoli3/FreeImage/commit/d866b72), [pull 55](https://github.com/danoli3/FreeImage/pull/55) | Submitted; status unspecified; SVN application unverified |
| [#176](https://sourceforge.net/p/freeimage/patches/176/) | CVE-2024-28584: NULL-check JPEG2000 image after opj_read_header | [commit 9286f7f](https://github.com/danoli3/FreeImage/commit/9286f7f), [pull 53](https://github.com/danoli3/FreeImage/pull/53) | Submitted; status unspecified; SVN application unverified |
| [#177](https://sourceforge.net/p/freeimage/patches/177/) | CVE-2024-28581: TARGA RLE IOCache at least one pixel wide | [commit 68af81b](https://github.com/danoli3/FreeImage/commit/68af81b), [pull 39](https://github.com/danoli3/FreeImage/pull/39) | Submitted; status unspecified; SVN application unverified |
| [#178](https://sourceforge.net/p/freeimage/patches/178/) | CVE-2024-28567: re-fetch TIFF ICC pointer after ExifIFD switch | [commit ce6fe58](https://github.com/danoli3/FreeImage/commit/ce6fe58), [pull 79](https://github.com/danoli3/FreeImage/pull/79) | Submitted; status unspecified; SVN application unverified |
| [#179](https://sourceforge.net/p/freeimage/patches/179/) | CVE-2021-33367: bound Exif 0th-IFD next-offset read | [commit 5e1e391](https://github.com/danoli3/FreeImage/commit/5e1e391) | Submitted; status unspecified; SVN application unverified |
| [#180](https://sourceforge.net/p/freeimage/patches/180/) | CVE-2024-28568/9029/28570/28573/28577/28578/28580: IPTC/Exif/XPM/RAS OOB | [commit d474400](https://github.com/danoli3/FreeImage/commit/d474400), [pull 96](https://github.com/danoli3/FreeImage/pull/96) | Submitted; status unspecified; SVN application unverified |
| [#181](https://sourceforge.net/p/freeimage/patches/181/) | CVE-2024-28566/28571/28572/28583: TIFF AssignPixel, JPEG src, MakerNote, XBM | [commit 8ebe8e5](https://github.com/danoli3/FreeImage/commit/8ebe8e5), [pull 99](https://github.com/danoli3/FreeImage/pull/99) | Submitted; status unspecified; SVN application unverified |
| [#182](https://sourceforge.net/p/freeimage/patches/182/) | CVE-2024-28574/28575/28576: replace bundled OpenJPEG 2.0.0 with 2.5.4 | [commit 229de19](https://github.com/danoli3/FreeImage/commit/229de19), [pull 68](https://github.com/danoli3/FreeImage/pull/68) | Submitted; status unspecified; SVN application unverified |
| [#183](https://sourceforge.net/p/freeimage/patches/183/) | CVE-2024-28562/28563/28564/28569: replace bundled OpenEXR 2.2 with 3.3.13 | [commit 248c6c9](https://github.com/danoli3/FreeImage/commit/248c6c9), [pull 69](https://github.com/danoli3/FreeImage/pull/69) | Superseded by #197; SVN application unverified |
| [#184](https://sourceforge.net/p/freeimage/patches/184/) | Prevent scanline pointer underflow in CUT RLE decoding | [commit 9d62eb2](https://github.com/danoli3/FreeImage/commit/9d62eb2) | Submitted with verified attachment; SVN application unverified |
| [#185](https://sourceforge.net/p/freeimage/patches/185/) | Reject three-plane PCX images with insufficient bytes per plane | [commit a4473a4](https://github.com/danoli3/FreeImage/commit/a4473a4) | Submitted with verified attachment; SVN application unverified |
| [#186](https://sourceforge.net/p/freeimage/patches/186/) | Preserve DDS DXT dimensions and decode trailing partial blocks | [commit bf820dc](https://github.com/danoli3/FreeImage/commit/bf820dc) | Submitted with verified attachment; SVN application unverified |
| [#187](https://sourceforge.net/p/freeimage/patches/187/) | Select RGB565 conversion using the distinguishing green mask | [commit 1a9ccb5](https://github.com/danoli3/FreeImage/commit/1a9ccb5) | Submitted with verified attachment; SVN application unverified |
| [#188](https://sourceforge.net/p/freeimage/patches/188/) | Accept BMP information-header sizes already handled by the loader | [commit 7646cfe](https://github.com/danoli3/FreeImage/commit/7646cfe) | Submitted with verified attachment; SVN application unverified |
| [#189](https://sourceforge.net/p/freeimage/patches/189/) | Accept trailing BMP RLE8 end-of-bitmap after the final end-of-line | [commit b9c6227](https://github.com/danoli3/FreeImage/commit/b9c6227) | Submitted with verified attachment; SVN application unverified |
| [#190](https://sourceforge.net/p/freeimage/patches/190/) | Complete the OpenJPEG 2.5.4 Windows project source list (follow-up to #182) | [patch and validation](upstream-patches/2026-10-13-vendors/openjpeg-2.5.4-project.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#191](https://sourceforge.net/p/freeimage/patches/191/) | Update bundled zlib from 1.2.13 to 1.3.2 | [patch and validation](upstream-patches/2026-10-13-vendors/zlib-1.3.2.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#192](https://sourceforge.net/p/freeimage/patches/192/) | Update bundled libpng from 1.6.39 to 1.6.59 | [patch and validation](upstream-patches/2026-10-13-vendors/libpng-1.6.59.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#193](https://sourceforge.net/p/freeimage/patches/193/) | Update bundled WebP from 1.2.1 to 1.6.0 | [patch and validation](upstream-patches/2026-10-13-vendors/webp-1.6.0.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#194](https://sourceforge.net/p/freeimage/patches/194/) | Update bundled libtiff from 4.5.0 to 4.7.2 | [patch and validation](upstream-patches/2026-10-13-vendors/libtiff-4.7.2.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#195](https://sourceforge.net/p/freeimage/patches/195/) | Update bundled LibRawLite from 0.21.1 to 0.22.2 | [patch and validation](upstream-patches/2026-10-13-vendors/libraw-0.22.2.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#196](https://sourceforge.net/p/freeimage/patches/196/) | Update bundled IJG JPEG from 9d to 10.0 | [patch and validation](upstream-patches/2026-10-13-vendors/jpeg-10.0.md) | Submitted; upload SHA-256 verified; SVN application unverified |
| [#197](https://sourceforge.net/p/freeimage/patches/197/) | Replace #183 with OpenEXR 3.5.2, Imath 3.2.3 and libdeflate 1.26 | [patch and validation](upstream-patches/2026-10-13-vendors/openexr-3.5.2.md) | Submitted; upload SHA-256 verified; SVN application unverified |

Ticket [#169](https://sourceforge.net/p/freeimage/patches/169/) is an empty accidental ICO stub without an attachment; #170 explicitly replaces it. Do not count both as separate submitted fixes.

Submission ordering from the ticket descriptions: #179 before later Exif changes; #180 before #181; #178 before #181's TIFF changes; #182 before #183. The original vendor submissions are OpenJPEG 2.5.4 and OpenEXR 3.3.13. New #190 supplements #182 with its Windows project list; new #197 replaces #183 with OpenEXR 3.5.2 and updated auxiliary libraries. The old #183 attachment is retained for history, not the current series.

## Latest submitted batch

[Six focused submissions](upstream-patches/2026-10-13/README.md) were posted on 2026-10-13 as #184–189: CUT, PCX, DDS DXT partial blocks, RGB565 conversion, BMP extended headers and BMP RLE8 termination. Ticket creation and attached ZIP bundles were verified through the live API. These fixes reproduce on unpatched r1911 files and pass after the fixes; see the batch README for sanitizer coverage and duplicate-review limits.

Status: **submitted; upstream SVN application unverified**. There are now 28 verified submission tickets in this tracker (including the superseded #183 archive), excluding the empty #169 stub.

## Submitted vendor follow-ups

[The vendor series](upstream-patches/2026-10-13-vendors/README.md) was submitted on 2026-10-13 as #190–197. Every uploaded gzip patch was downloaded and its SHA-256 compared with the local artifact.

Apply the existing #182 source upgrade, then #190 (OpenJPEG Windows project metadata), #191 (zlib), #192 (libpng), #193 (WebP), #194 (libtiff), #195 (LibRaw), #196 (IJG JPEG), and #197 (OpenEXR/Imath/libdeflate plus auxiliary codecs). **#197 replaces the older #183 attachment; do not apply both.** These are an ordered series because shared build/project files change at each step.

The fully patched SVN tree builds and passes the codec round trips described in the batch README. VS project XML and compile-source paths were checked, but the adapted SVN patches have not been built on Windows/Android/Linux/x86, and real-camera RAW decoding was not tested. An existing jxrlib missing-declaration problem required a validation-only compiler header on macOS; it is documented and not part of these patches.

The original #182/#183 descriptions remain unchanged: an edit was denied (HTTP 403), and the REST comment endpoints returned 404. The new tickets explicitly reference their originals. No acceptance into upstream SVN has been verified.

## Merged fixes needing submission checks

These are candidates, not a claim that no submission exists. The complete ticket-title inventory below was checked, but older attachments and discussion comments still require comparison. Prepare small patches against current upstream SVN rather than attaching an entire fork diff.

| Priority | Change merged here | Evidence | Next action |
| --- | --- | --- | --- |
| High | PICT unpacking and color-table bounds | [051e9da](https://github.com/danoli3/FreeImage/commit/051e9da), [a3eb964](https://github.com/danoli3/FreeImage/commit/a3eb964) | Compare #149/#143 and older attachments; attach missing cases with ASan evidence. |
| Medium | Plugin format-ID stability when codecs are disabled | [abf026e](https://github.com/danoli3/FreeImage/commit/abf026e) | Submit a focused core patch with enabled/disabled codec tests if absent. |
| Medium | Thread-safe metadata formatting | [735872d](https://github.com/danoli3/FreeImage/commit/735872d) | Check upstream implementation and submission history. |
| Medium | Sequential GIF playback cache | [8395cea](https://github.com/danoli3/FreeImage/commit/8395cea) | Compare #92; determine whether our patch supersedes it. |
| Medium | CR3/X3F support and CR3 detection | [PR #147](https://github.com/danoli3/FreeImage/pull/147) | Preserve contributor attribution; check LibRaw baseline before proposing upstream. |
| Maintenance | CMake, system dependency and installation improvements | [PR #128](https://github.com/danoli3/FreeImage/pull/128), [PR #160](https://github.com/danoli3/FreeImage/pull/160) | Compare upstream #141; #160 is separate PR work, not part of the audited master baseline. |

Changes inside vendored libraries may belong with their original maintainers too. Track that separately from FreeImage integration: Android API 25 OpenEXR allocation ([#145](https://github.com/danoli3/FreeImage/pull/145)), strict-C11 libdeflate ARM declarations ([#144](https://github.com/danoli3/FreeImage/pull/144)), Windows OpenEXR file APIs ([#151](https://github.com/danoli3/FreeImage/pull/151)), and OpenJPH x86 work ([#150](https://github.com/danoli3/FreeImage/pull/150), [#155](https://github.com/danoli3/FreeImage/pull/155)). Verify upstream provenance and PR merge state before claiming these as new submissions or merged fixes.

## Existing upstream patches needing a source audit

Start with #167 (Canon MakerNote; compare our #181), #159 (PSD RLE; compare #174), #131 (TIFF ICC; compare #178), #163 (RAS palette; compare commit 8445931), and #92 (GIF playback; compare commit 8395cea). Similar titles do not prove identical fixes. Then inspect #164/#165, TIFF metadata/thumbnail patches, big-endian fixes, and #137 AVIF alongside the [current candidate catalog](upstream-patches/README.md).

## Updating this tracker

For each fix, record the fork PR/commit, affected source files, reproducer and validation, first released version, upstream ticket and attachment, target SVN revision and patch dependencies. After submission, keep its ticket link. After acceptance, add the actual upstream revision and verify its diff before changing the status. Check for duplicates before creating tickets. New SourceForge submissions are a separate action; this documentation update does not post any tickets.

## Complete upstream ticket inventory

All 182 ticket numbers returned by the live API before submission, plus the fourteen created and verified tickets (#184–197): 196 tickets total. Historical titles alone do not establish whether the fix is applied here or upstream. Unless matched above, the disposition is **needs audit**. This includes historical closed tickets because the REST index enumerates the whole tracker, unlike the default HTML view.

| Ticket | Indexed subject | Disposition |
| --- | --- | --- |
| [#197](https://sourceforge.net/p/freeimage/patches/197/) | Replace #183 with OpenEXR 3.5.2, Imath 3.2.3 and libdeflate 1.26 | Submitted; SVN application unverified |
| [#196](https://sourceforge.net/p/freeimage/patches/196/) | Update bundled IJG JPEG from 9d to 10.0 | Submitted; SVN application unverified |
| [#195](https://sourceforge.net/p/freeimage/patches/195/) | Update bundled LibRawLite from 0.21.1 to 0.22.2 | Submitted; SVN application unverified |
| [#194](https://sourceforge.net/p/freeimage/patches/194/) | Update bundled libtiff from 4.5.0 to 4.7.2 | Submitted; SVN application unverified |
| [#193](https://sourceforge.net/p/freeimage/patches/193/) | Update bundled WebP from 1.2.1 to 1.6.0 | Submitted; SVN application unverified |
| [#192](https://sourceforge.net/p/freeimage/patches/192/) | Update bundled libpng from 1.6.39 to 1.6.59 | Submitted; SVN application unverified |
| [#191](https://sourceforge.net/p/freeimage/patches/191/) | Update bundled zlib from 1.2.13 to 1.3.2 | Submitted; SVN application unverified |
| [#190](https://sourceforge.net/p/freeimage/patches/190/) | Complete the OpenJPEG 2.5.4 Windows project source list (follow-up to #182) | Submitted; SVN application unverified |
| [#189](https://sourceforge.net/p/freeimage/patches/189/) | Accept trailing BMP RLE8 end-of-bitmap after the final end-of-line | Submitted; SVN application unverified |
| [#188](https://sourceforge.net/p/freeimage/patches/188/) | Accept BMP information-header sizes already handled by the loader | Submitted; SVN application unverified |
| [#187](https://sourceforge.net/p/freeimage/patches/187/) | Select RGB565 conversion using the distinguishing green mask | Submitted; SVN application unverified |
| [#186](https://sourceforge.net/p/freeimage/patches/186/) | Preserve DDS DXT dimensions and decode trailing partial blocks | Submitted; SVN application unverified |
| [#185](https://sourceforge.net/p/freeimage/patches/185/) | Reject three-plane PCX images with insufficient bytes per plane | Submitted; SVN application unverified |
| [#184](https://sourceforge.net/p/freeimage/patches/184/) | Prevent scanline pointer underflow in CUT RLE decoding | Submitted; SVN application unverified |
| [#183](https://sourceforge.net/p/freeimage/patches/183/) | CVE-2024-28562/28563/28564/28569: replace bundled OpenEXR 2.2 with 3.3.13 | Superseded by #197; SVN application unverified |
| [#182](https://sourceforge.net/p/freeimage/patches/182/) | CVE-2024-28574/28575/28576: replace bundled OpenJPEG 2.0.0 with 2.5.4 | Submitted; SVN application unverified |
| [#181](https://sourceforge.net/p/freeimage/patches/181/) | CVE-2024-28566/28571/28572/28583: TIFF AssignPixel, JPEG src, MakerNote, XBM | Submitted; SVN application unverified |
| [#180](https://sourceforge.net/p/freeimage/patches/180/) | CVE-2024-28568/9029/28570/28573/28577/28578/28580: IPTC/Exif/XPM/RAS OOB | Submitted; SVN application unverified |
| [#179](https://sourceforge.net/p/freeimage/patches/179/) | CVE-2021-33367: bound Exif 0th-IFD next-offset read | Submitted; SVN application unverified |
| [#178](https://sourceforge.net/p/freeimage/patches/178/) | CVE-2024-28567: re-fetch TIFF ICC pointer after ExifIFD switch | Submitted; SVN application unverified |
| [#177](https://sourceforge.net/p/freeimage/patches/177/) | CVE-2024-28581: TARGA RLE IOCache at least one pixel wide | Submitted; SVN application unverified |
| [#176](https://sourceforge.net/p/freeimage/patches/176/) | CVE-2024-28584: NULL-check JPEG2000 image after opj_read_header | Submitted; SVN application unverified |
| [#175](https://sourceforge.net/p/freeimage/patches/175/) | CVE-2024-28579 / CVE-2024-28582: reject overflowing HDR RLE scanline counts | Submitted; SVN application unverified |
| [#174](https://sourceforge.net/p/freeimage/patches/174/) | CVE-2020-24294 / CVE-2024-28565: clamp psdParser::UnpackRLE src and dst | Submitted; SVN application unverified |
| [#173](https://sourceforge.net/p/freeimage/patches/173/) | CVE-2020-22524: fix PFM integer overflow in line buffer allocation | Submitted; SVN application unverified |
| [#172](https://sourceforge.net/p/freeimage/patches/172/) | CVE-2020-24295: bound psdParser::ReadImageLine destination writes | Submitted; SVN application unverified |
| [#171](https://sourceforge.net/p/freeimage/patches/171/) | CVE-2020-24293: bound psdThumbnail::Read packed-row copy | Submitted; SVN application unverified |
| [#170](https://sourceforge.net/p/freeimage/patches/170/) | CVE-2020-24292: reject illegal ICO BITMAPINFOHEADER bpp/size | Submitted; SVN application unverified |
| [#169](https://sourceforge.net/p/freeimage/patches/169/) | CVE-2020-24292: reject illegal ICO BITMAPINFOHEADER bpp/size | Empty duplicate; use #170 |
| [#168](https://sourceforge.net/p/freeimage/patches/168/) | Any chance for a WinARM64 build of FreeImage.dll? | Needs audit |
| [#167](https://sourceforge.net/p/freeimage/patches/167/) | FreeImage 3.18.0 Canon MakerNote out-of-bounds read | Needs audit |
| [#166](https://sourceforge.net/p/freeimage/patches/166/) | FreeImage_Save fails to save in DDS file format | Needs audit |
| [#165](https://sourceforge.net/p/freeimage/patches/165/) | Fix for #362 (CVE-2023-47997) | Needs audit |
| [#164](https://sourceforge.net/p/freeimage/patches/164/) | Fix for #360 (CVE-2023-47995) | Needs audit |
| [#163](https://sourceforge.net/p/freeimage/patches/163/) | Add palette pointer check in PluginRAS.cpp | Needs audit |
| [#162](https://sourceforge.net/p/freeimage/patches/162/) | Add check for psd file channels. | Needs audit |
| [#161](https://sourceforge.net/p/freeimage/patches/161/) | Two patch fix out of bounds. | Needs audit |
| [#160](https://sourceforge.net/p/freeimage/patches/160/) | Limit input psd image color depth. | Needs audit |
| [#159](https://sourceforge.net/p/freeimage/patches/159/) | psdParser::UnpackRLE() memory out of bounds. | Needs audit |
| [#158](https://sourceforge.net/p/freeimage/patches/158/) | Numeric Trunctation at `misc_parsers.cpp:257` | Needs audit |
| [#157](https://sourceforge.net/p/freeimage/patches/157/) | Fix integer wrap around and segmentation fault in Exif.cpp | Needs audit |
| [#156](https://sourceforge.net/p/freeimage/patches/156/) | Fixed some issues with the EXR file reading | Needs audit |
| [#155](https://sourceforge.net/p/freeimage/patches/155/) | Bump LibRaw version | Needs audit |
| [#154](https://sourceforge.net/p/freeimage/patches/154/) | Fix out of bounds read in PluginBMP | Needs audit |
| [#153](https://sourceforge.net/p/freeimage/patches/153/) | patch for building 3.18.0 on big-endian hosts | Needs audit |
| [#152](https://sourceforge.net/p/freeimage/patches/152/) | Bugfix for #346 - Fix heap corruption when reading TIFFTAG_RICHTIFFIPTC | Needs audit |
| [#151](https://sourceforge.net/p/freeimage/patches/151/) | Updates OpenEXR to version 3.1.3 | Needs audit |
| [#150](https://sourceforge.net/p/freeimage/patches/150/) | Avoids clamping to [0..1] when converting bitmaps from one Float format to another | Needs audit |
| [#149](https://sourceforge.net/p/freeimage/patches/149/) | Fix 5 crashes revealed by oss-fuzz | Needs audit |
| [#148](https://sourceforge.net/p/freeimage/patches/148/) | Bugfix for #340 (Calling FreeImage_Paste fails on very large images) | Needs audit |
| [#147](https://sourceforge.net/p/freeimage/patches/147/) | Bugfix for #339 | Needs audit |
| [#146](https://sourceforge.net/p/freeimage/patches/146/) | RAII for easier memory management | Needs audit |
| [#145](https://sourceforge.net/p/freeimage/patches/145/) | Build patches used in homebrew | Needs audit |
| [#144](https://sourceforge.net/p/freeimage/patches/144/) | E2K: fixed build by MCST lcc compiler (MCST Elbrus 2000 architecture) | Needs audit |
| [#143](https://sourceforge.net/p/freeimage/patches/143/) | Bugfix for #299,#303,#304,#305,#308,#309,#310, #320  | Needs audit |
| [#142](https://sourceforge.net/p/freeimage/patches/142/) | Bugfix for bugs #306 #307 as well as problem from patch #139 | Needs audit |
| [#141](https://sourceforge.net/p/freeimage/patches/141/) | CMake support patch | Needs audit |
| [#140](https://sourceforge.net/p/freeimage/patches/140/) | Few patches from vspkg | Needs audit |
| [#139](https://sourceforge.net/p/freeimage/patches/139/) | Solve the problem of PluginTARGA | Needs audit |
| [#138](https://sourceforge.net/p/freeimage/patches/138/) | TIFF Deflate compression fixes | Needs audit |
| [#137](https://sourceforge.net/p/freeimage/patches/137/) | New format AVIF (AV1 Image File) | Needs audit |
| [#136](https://sourceforge.net/p/freeimage/patches/136/) | Implement TFreeWinBitmap.Draw with a custom source rectangle. | Needs audit |
| [#135](https://sourceforge.net/p/freeimage/patches/135/) | fix big endian builds | Needs audit |
| [#134](https://sourceforge.net/p/freeimage/patches/134/) | FreeImage_LoadFromMemory is unnecessarily slow | Needs audit |
| [#133](https://sourceforge.net/p/freeimage/patches/133/) | Need an extra EXIF tag for TIFF | Needs audit |
| [#132](https://sourceforge.net/p/freeimage/patches/132/) | Additional RAW metadata | Needs audit |
| [#131](https://sourceforge.net/p/freeimage/patches/131/) | Tiff embedded icc profiles are overwritten before use | Needs audit |
| [#130](https://sourceforge.net/p/freeimage/patches/130/) | freeimage fails to build against jpeg-9c | Needs audit |
| [#129](https://sourceforge.net/p/freeimage/patches/129/) | Error-checking is lacking | Needs audit |
| [#128](https://sourceforge.net/p/freeimage/patches/128/) | Add const-poisoning | Needs audit |
| [#127](https://sourceforge.net/p/freeimage/patches/127/) | Fixed strange error handling in PNG plugin | Needs audit |
| [#126](https://sourceforge.net/p/freeimage/patches/126/) | Visual Studio 2015 project files | Needs audit |
| [#125](https://sourceforge.net/p/freeimage/patches/125/) | Additional FIF_RAW Flags | Needs audit |
| [#124](https://sourceforge.net/p/freeimage/patches/124/) | Delphi 10.1 Berlin support for wrapper | Needs audit |
| [#123](https://sourceforge.net/p/freeimage/patches/123/) | Bugfix: Tiff thumbnails fails to load if exif is present | Needs audit |
| [#122](https://sourceforge.net/p/freeimage/patches/122/) | Refactor: Streamline MultiPage and CacheFile | Needs audit |
| [#121](https://sourceforge.net/p/freeimage/patches/121/) | PluginDDS Bug Fix | Needs audit |
| [#120](https://sourceforge.net/p/freeimage/patches/120/) | Bugfix: Image View broken save as BMP | Needs audit |
| [#119](https://sourceforge.net/p/freeimage/patches/119/) | Bugfix: Cloned bitmap "view" still points to original external bitmap | Needs audit |
| [#118](https://sourceforge.net/p/freeimage/patches/118/) | Bugfix: ColorType is incorrect for 16bit CMYK | Needs audit |
| [#117](https://sourceforge.net/p/freeimage/patches/117/) | Bugfix: Color Profile is actually still there | Needs audit |
| [#116](https://sourceforge.net/p/freeimage/patches/116/) | Jpeg CMYK Save support | Needs audit |
| [#115](https://sourceforge.net/p/freeimage/patches/115/) | Fixes for FreeImage - iOS, OSX, arm64 and x86_64 | Needs audit |
| [#114](https://sourceforge.net/p/freeimage/patches/114/) | OS X 10.11.3, make is not working. | Needs audit |
| [#113](https://sourceforge.net/p/freeimage/patches/113/) | Delphi Wrapper updated for 3.18.0 / Seattle | Needs audit |
| [#112](https://sourceforge.net/p/freeimage/patches/112/) | Wrong colors in PNG image | Needs audit |
| [#111](https://sourceforge.net/p/freeimage/patches/111/) | PSD write support | Needs audit |
| [#110](https://sourceforge.net/p/freeimage/patches/110/) | fixes for GCC 5.0 | Needs audit |
| [#109](https://sourceforge.net/p/freeimage/patches/109/) | Delphi wrapper 3.17.0 update | Needs audit |
| [#108](https://sourceforge.net/p/freeimage/patches/108/) | Visual Studio 2015 supports snprintf | Needs audit |
| [#107](https://sourceforge.net/p/freeimage/patches/107/) | FreeImage_ConvertToRawBits support for transparency from palletized to 32bit | Needs audit |
| [#106](https://sourceforge.net/p/freeimage/patches/106/) | Compile FreeImage on Mac OSX 10.10 Yosemite | Needs audit |
| [#105](https://sourceforge.net/p/freeimage/patches/105/) | Minor bug in several Makefiles  | Needs audit |
| [#104](https://sourceforge.net/p/freeimage/patches/104/) | New function: FreeImage_RescaleEx | Needs audit |
| [#103](https://sourceforge.net/p/freeimage/patches/103/) | Fixed PNG plugin handling of 24-bit RGB + tRNS chunk images | Needs audit |
| [#102](https://sourceforge.net/p/freeimage/patches/102/) | Fixed PNG plugin handling of 16-bit grayscale + 16-bit alpha images | Needs audit |
| [#101](https://sourceforge.net/p/freeimage/patches/101/) | New function: FreeImage_ConvertToRGBAF | Needs audit |
| [#100](https://sourceforge.net/p/freeimage/patches/100/) | New function: FreeImage_GetMemorySize | Needs audit |
| [#99](https://sourceforge.net/p/freeimage/patches/99/) | New Lossless Fast Pseudo-Quantization algorithm | Needs audit |
| [#98](https://sourceforge.net/p/freeimage/patches/98/) | New function: FreeImage_CreateView | Needs audit |
| [#97](https://sourceforge.net/p/freeimage/patches/97/) | Explicit definition of endianness and color order | Needs audit |
| [#96](https://sourceforge.net/p/freeimage/patches/96/) | Fixes for FreeImage - iOS and arm64 | Needs audit |
| [#95](https://sourceforge.net/p/freeimage/patches/95/) | Infinite Loop Determining File Type for Truncated ANI (RIFF) | Needs audit |
| [#93](https://sourceforge.net/p/freeimage/patches/93/) | Segfault on Corrupt Animated GIF | Needs audit |
| [#92](https://sourceforge.net/p/freeimage/patches/92/) | Increase Speed of GIF_PLAYBACK Successive Frames | Needs audit |
| [#91](https://sourceforge.net/p/freeimage/patches/91/) | Delphi wrapper version 3.16.1 | Needs audit |
| [#90](https://sourceforge.net/p/freeimage/patches/90/) | Patch overflow bug on CalculateScanline | Needs audit |
| [#89](https://sourceforge.net/p/freeimage/patches/89/) | FreeImage CompileTime Configuration Support | Needs audit |
| [#88](https://sourceforge.net/p/freeimage/patches/88/) | Massively improved JPEG Transform | Needs audit |
| [#87](https://sourceforge.net/p/freeimage/patches/87/) | NetBSD and memalign | Needs audit |
| [#86](https://sourceforge.net/p/freeimage/patches/86/) | Create image from user bits without copy of pixel data | Needs audit |
| [#85](https://sourceforge.net/p/freeimage/patches/85/) | Minor issues in Resize resulting in lost performance | Needs audit |
| [#84](https://sourceforge.net/p/freeimage/patches/84/) | Recognize rgb as valid SGI files | Needs audit |
| [#83](https://sourceforge.net/p/freeimage/patches/83/) | Delphi wrapper update | Needs audit |
| [#82](https://sourceforge.net/p/freeimage/patches/82/) | TGA file read failure on valid TGA file | Needs audit |
| [#81](https://sourceforge.net/p/freeimage/patches/81/) | include memset.h for ImfAutoArray.h for G++ | Needs audit |
| [#80](https://sourceforge.net/p/freeimage/patches/80/) | include string.h for memset | Needs audit |
| [#79](https://sourceforge.net/p/freeimage/patches/79/) | Visual Studio 2012 Solution supporting batch builds | Needs audit |
| [#78](https://sourceforge.net/p/freeimage/patches/78/) | Suggested changes for Plugin.cpp | Needs audit |
| [#77](https://sourceforge.net/p/freeimage/patches/77/) | delphi wrapper 3.15.3 | Needs audit |
| [#76](https://sourceforge.net/p/freeimage/patches/76/) | Override builtin Plugins with a Local (or external) one | Needs audit |
| [#75](https://sourceforge.net/p/freeimage/patches/75/) | add missing headers for mingw | Needs audit |
| [#74](https://sourceforge.net/p/freeimage/patches/74/) | Use correct mime for file types | Needs audit |
| [#73](https://sourceforge.net/p/freeimage/patches/73/) | EXIF_MAIN write support for PluginTIFF | Needs audit |
| [#72](https://sourceforge.net/p/freeimage/patches/72/) | io.h to asm/io.h | Needs audit |
| [#71](https://sourceforge.net/p/freeimage/patches/71/) | libjpeg-turbo for Linux | Needs audit |
| [#70](https://sourceforge.net/p/freeimage/patches/70/) | tiff without samplesperpixel or bitspersample | Needs audit |
| [#69](https://sourceforge.net/p/freeimage/patches/69/) | Update Delphi Wrapper for 64 bit | Needs audit |
| [#68](https://sourceforge.net/p/freeimage/patches/68/) | Add half size flag to raw decoding | Needs audit |
| [#67](https://sourceforge.net/p/freeimage/patches/67/) | update Delphi wrapper for 3.15.0 | Needs audit |
| [#66](https://sourceforge.net/p/freeimage/patches/66/) | little update for delphi examples | Needs audit |
| [#65](https://sourceforge.net/p/freeimage/patches/65/) | Callbacks support | Needs audit |
| [#64](https://sourceforge.net/p/freeimage/patches/64/) | delphi wrapper update 3.14.1 | Needs audit |
| [#63](https://sourceforge.net/p/freeimage/patches/63/) | fix missing header | Needs audit |
| [#62](https://sourceforge.net/p/freeimage/patches/62/) | Read 16 bit BMPs Correctly | Needs audit |
| [#61](https://sourceforge.net/p/freeimage/patches/61/) | 32 and 24 bit conversion enhancements | Needs audit |
| [#60](https://sourceforge.net/p/freeimage/patches/60/) | Fix leaks in FreeImage_OpenMultiBitmapFromHandle() | Needs audit |
| [#59](https://sourceforge.net/p/freeimage/patches/59/) | Several bugs in FreeImage_CloseMultiBitmap() | Needs audit |
| [#58](https://sourceforge.net/p/freeimage/patches/58/) | Fix lots of leaks in FreeImage_OpenMultiBitmap() | Needs audit |
| [#57](https://sourceforge.net/p/freeimage/patches/57/) | FreeImage_GetOriginalBPP | Needs audit |
| [#56](https://sourceforge.net/p/freeimage/patches/56/) | Thumbnail support | Needs audit |
| [#55](https://sourceforge.net/p/freeimage/patches/55/) | delphi wrapper update | Needs audit |
| [#54](https://sourceforge.net/p/freeimage/patches/54/) | FreeImage_CloseMultiBitmap() without saving to file | Needs audit |
| [#53](https://sourceforge.net/p/freeimage/patches/53/) | Fix TIFFSetField call for TIFFTAG_PAGENUMBER | Needs audit |
| [#52](https://sourceforge.net/p/freeimage/patches/52/) | Extended Tiff Plugin | Needs audit |
| [#51](https://sourceforge.net/p/freeimage/patches/51/) | VS2005 x64 sln and prj update | Needs audit |
| [#50](https://sourceforge.net/p/freeimage/patches/50/) | Refactored PSD image data loading | Needs audit |
| [#49](https://sourceforge.net/p/freeimage/patches/49/) | FreeImage_CloseMultiBitmap() without saving to file | Needs audit |
| [#48](https://sourceforge.net/p/freeimage/patches/48/) | Support for additional channels (beyond RGBA) | Needs audit |
| [#47](https://sourceforge.net/p/freeimage/patches/47/) | Fix for unnecessary EXR loading limitation | Needs audit |
| [#46](https://sourceforge.net/p/freeimage/patches/46/) | Updated Example for Linux | Needs audit |
| [#45](https://sourceforge.net/p/freeimage/patches/45/) | FreeImage_ConvertTo32Bits performance hit | Needs audit |
| [#44](https://sourceforge.net/p/freeimage/patches/44/) | fix for PluginGIF saving corrupeted image (test image) | Needs audit |
| [#43](https://sourceforge.net/p/freeimage/patches/43/) | JPEG plugin standard optimized encoding support | Needs audit |
| [#42](https://sourceforge.net/p/freeimage/patches/42/) | JPG plugin comment storing improvements | Needs audit |
| [#41](https://sourceforge.net/p/freeimage/patches/41/) | fix for PluginGIF saving corrupeted image | Needs audit |
| [#40](https://sourceforge.net/p/freeimage/patches/40/) | Refactoring TARGA plugin (cont)  | Needs audit |
| [#39](https://sourceforge.net/p/freeimage/patches/39/) | Don't use libmng_data.h | Needs audit |
| [#38](https://sourceforge.net/p/freeimage/patches/38/) | Quickie to extract first frame of MNG | Needs audit |
| [#37](https://sourceforge.net/p/freeimage/patches/37/) | [TGA]Corrupt rle count fix | Needs audit |
| [#36](https://sourceforge.net/p/freeimage/patches/36/) | Quickie to extract first frame of MNG | Needs audit |
| [#35](https://sourceforge.net/p/freeimage/patches/35/) | Quickie to extract first frame of MNG | Needs audit |
| [#34](https://sourceforge.net/p/freeimage/patches/34/) | Quickie to extract first frame of MNG | Needs audit |
| [#33](https://sourceforge.net/p/freeimage/patches/33/) | Quickie to extract first frame of MNG | Needs audit |
| [#32](https://sourceforge.net/p/freeimage/patches/32/) | Crash on corrupted tags | Needs audit |
| [#31](https://sourceforge.net/p/freeimage/patches/31/) | Addition of FreeImage_LoadHeader function group | Needs audit |
| [#30](https://sourceforge.net/p/freeimage/patches/30/) | VS 2008 and X64 Windows builds | Needs audit |
| [#29](https://sourceforge.net/p/freeimage/patches/29/) | Background filling routines patch | Needs audit |
| [#28](https://sourceforge.net/p/freeimage/patches/28/) | Fixed some typos in README.minGW | Needs audit |
| [#27](https://sourceforge.net/p/freeimage/patches/27/) | Fixed some typos in README.minGW | Needs audit |
| [#26](https://sourceforge.net/p/freeimage/patches/26/) | Macro DEPRECATE fails on x64 | Needs audit |
| [#25](https://sourceforge.net/p/freeimage/patches/25/) | MinGW Makefile | Needs audit |
| [#24](https://sourceforge.net/p/freeimage/patches/24/) | New 'monochrome' function fixes ConvertToStandardType and... | Needs audit |
| [#23](https://sourceforge.net/p/freeimage/patches/23/) | Reinhard05 fix-up | Needs audit |
| [#22](https://sourceforge.net/p/freeimage/patches/22/) | Open multipage bitmap from handle | Needs audit |
| [#21](https://sourceforge.net/p/freeimage/patches/21/) | Refactoring TARGA plugin | Needs audit |
| [#20](https://sourceforge.net/p/freeimage/patches/20/) | [PATCH] JPEG rotate_exif bugs Version 3.12.0 | Needs audit |
| [#19](https://sourceforge.net/p/freeimage/patches/19/) | fix for PluginBMP alignment bug while saving 16 or 24bit BMP | Needs audit |
| [#18](https://sourceforge.net/p/freeimage/patches/18/) | Patch to allow x64 build | Needs audit |
| [#17](https://sourceforge.net/p/freeimage/patches/17/) | Upgrade to VS2008 | Needs audit |
| [#16](https://sourceforge.net/p/freeimage/patches/16/) | [PATCH]  Fix handling of frame disposal in GIF_PLAYBACK mode | Needs audit |
| [#15](https://sourceforge.net/p/freeimage/patches/15/) | Added support for SGI Grayscale + Alpha pics | Needs audit |
| [#14](https://sourceforge.net/p/freeimage/patches/14/) | added PNG Interlacing and compression support | Needs audit |
| [#13](https://sourceforge.net/p/freeimage/patches/13/) | Fix : SGI files with 2 dimesions crash | Needs audit |
| [#12](https://sourceforge.net/p/freeimage/patches/12/) | Fix and clean OSX makefile | Needs audit |
| [#11](https://sourceforge.net/p/freeimage/patches/11/) | VC8 compatibility latest CVS | Needs audit |
| [#10](https://sourceforge.net/p/freeimage/patches/10/) | linux fixes | Needs audit |
| [#9](https://sourceforge.net/p/freeimage/patches/9/) | Fix for Bug in PluginGIF.cpp | Needs audit |
| [#8](https://sourceforge.net/p/freeimage/patches/8/) | FreeImagePlus const correctness, fipMemoryIO copy bug | Needs audit |
| [#7](https://sourceforge.net/p/freeimage/patches/7/) | DDS patch for bad DDS files | Needs audit |
| [#6](https://sourceforge.net/p/freeimage/patches/6/) | SGI plugin | Needs audit |
| [#5](https://sourceforge.net/p/freeimage/patches/5/) | dds memory leak | Needs audit |
| [#4](https://sourceforge.net/p/freeimage/patches/4/) | DDS crash fix - FreeImag 3.8.0 | Needs audit |
| [#3](https://sourceforge.net/p/freeimage/patches/3/) | Cygwin Makefile | Needs audit |
| [#2](https://sourceforge.net/p/freeimage/patches/2/) | fipImage construction bug | Needs audit |
| [#1](https://sourceforge.net/p/freeimage/patches/1/) | FloydSteinberg memory leak | Needs audit |
