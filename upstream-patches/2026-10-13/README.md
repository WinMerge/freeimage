# Prepared upstream submissions — 2026-10-13

Six separate tickets were submitted to SourceForge on 2026-10-13. Ticket creation and ZIP attachments were verified. **Upstream SVN acceptance remains unverified.**

| Patch / ticket text | Attachment | Validation |
| --- | --- | --- |
| [Prevent scanline pointer underflow in CUT RLE decoding](cut-rle-bounds.md) | [cut-rle-bounds.patch](cut-rle-bounds.patch), [cut-rle-bounds-submission.zip](cut-rle-bounds-submission.zip) | Baseline fails; patched case passes |
| [Reject three-plane PCX images with insufficient bytes per plane](pcx-plane-width.md) | [pcx-plane-width.patch](pcx-plane-width.patch), [pcx-plane-width-submission.zip](pcx-plane-width-submission.zip) | Baseline fails; patched case passes |
| [Preserve DDS DXT dimensions and decode trailing partial blocks](dds-partial-blocks.md) | [dds-partial-blocks.patch](dds-partial-blocks.patch), [dds-partial-blocks-submission.zip](dds-partial-blocks-submission.zip) | Baseline fails; patched case passes |
| [Select RGB565 conversion using the distinguishing green mask](rgb565-conversion.md) | [rgb565-conversion.patch](rgb565-conversion.patch), [rgb565-conversion-submission.zip](rgb565-conversion-submission.zip) | Baseline fails; patched case passes |
| [Accept BMP information-header sizes already handled by the loader](bmp-extended-headers.md) | [bmp-extended-headers.patch](bmp-extended-headers.patch), [bmp-extended-headers-submission.zip](bmp-extended-headers-submission.zip) | Baseline fails; patched case passes |
| [Accept trailing BMP RLE8 end-of-bitmap after the final end-of-line](bmp-rle8-terminator.md) | [bmp-rle8-terminator.patch](bmp-rle8-terminator.patch), [bmp-rle8-terminator-submission.zip](bmp-rle8-terminator-submission.zip) | Baseline fails; patched case passes |

Target: https://svn.code.sf.net/p/freeimage/svn/FreeImage/trunk at r1911. Patch paths use `-p0`; original source encodings and line endings were retained. The two BMP patches work independently and together.

[validation.json](validation.json) records process results. [regression.cpp](regression.cpp) supplies crafted cases. ASan covered the edited translation units and harness; the supporting fork archive was not instrumented. CUT has a confirmed baseline ASan failure. PCX rejection is a functional regression test for an uninitialized-read bug, not an ASan proof of initialization. Both existing fork CTest suites passed independently.

Verified upstream tickets:

- [cut-rle-bounds: #184](https://sourceforge.net/p/freeimage/patches/184/)
- [pcx-plane-width: #185](https://sourceforge.net/p/freeimage/patches/185/)
- [dds-partial-blocks: #186](https://sourceforge.net/p/freeimage/patches/186/)
- [rgb565-conversion: #187](https://sourceforge.net/p/freeimage/patches/187/)
- [bmp-extended-headers: #188](https://sourceforge.net/p/freeimage/patches/188/)
- [bmp-rle8-terminator: #189](https://sourceforge.net/p/freeimage/patches/189/)

Contributor credits are included in each ticket. Ticket links are recorded in UPSTREAM_PATCHES.md; acceptance still requires an upstream SVN revision. The Markdown files retain the exact ticket text used for submission.
