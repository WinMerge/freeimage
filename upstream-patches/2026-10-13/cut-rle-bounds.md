# Prevent scanline pointer underflow in CUT RLE decoding

Applies to FreeImage SVN trunk r1911, checked 2026-10-13.

A premature zero-count row marker decrements the scanline pointer below the allocated bitmap. A following repeat packet writes outside the allocation. The patch stops decoding before advancing below the first scanline. It preserves the existing partial-image return behavior.

Original contributor: Jorge Barredo Ferreira. License: retain the original FreeImage file license (FIPL/GPL).
Fork fix: https://github.com/danoli3/FreeImage/commit/9d62eb2

## Verification

The crafted width=0x230b, height=1 input produces an ASan heap-buffer-overflow with the SVN loader; the patched loader completes without a report.

The attached regression harness builds against the public FreeImage API. All six cases fail against unpatched r1911 versions of the affected files and pass after this batch. Each edited translation unit and the harness was built with `-fsanitize=address -g`, linked to the existing 3.19.21 core archive on macOS ARM64. The rest of that archive was not ASan-instrumented: this verifies the targeted code, not a full sanitized upstream build. The fork's existing MainTestSuite and VendorCodecTest also pass independently.

## Apply

Attach `cut-rle-bounds.patch` and `cut-rle-bounds-submission.zip` to this ticket. The ZIP contains the patch, harness and this description.

From the upstream source root:

```sh
patch --dry-run -p0 < cut-rle-bounds.patch
patch -p0 < cut-rle-bounds.patch
```

Run `regression cut-rle-bounds` after linking regression.cpp to the built library. The harness returns 0 for the corrected behavior, 1 for a regression and 2 for usage errors. An ASan abort also indicates failure for the CUT baseline.

All patches dry-run successfully on pristine r1911. The two BMP patches also apply together. No dependency upgrades or ABI changes are needed.

The live ticket index and relevant attachments from #121/#143/#149/#154 were inspected. No equivalent patch was identified in that review; this is not an exhaustive audit of every historical discussion or attachment.
