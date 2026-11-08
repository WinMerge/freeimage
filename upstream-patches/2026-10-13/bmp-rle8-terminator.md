# Accept trailing BMP RLE8 end-of-bitmap after the final end-of-line

Applies to FreeImage SVN trunk r1911, checked 2026-10-13.

Read the final end-of-bitmap marker even when the final end-of-line has advanced scanline to height. Validate the row before both absolute and encoded pixel writes so an extra data packet cannot write past the bitmap.

Original contributor: Dan Rosser. License: retain the original FreeImage file license (FIPL/GPL).
Fork fix: https://github.com/danoli3/FreeImage/commit/b9c6227

## Verification

A valid one-row stream with a trailing end-of-bitmap marker fails before this fix and loads after it. A data run following the final end-of-line is still rejected.

The attached regression harness builds against the public FreeImage API. All six cases fail against unpatched r1911 versions of the affected files and pass after this batch. Each edited translation unit and the harness was built with `-fsanitize=address -g`, linked to the existing 3.19.21 core archive on macOS ARM64. The rest of that archive was not ASan-instrumented: this verifies the targeted code, not a full sanitized upstream build. The fork's existing MainTestSuite and VendorCodecTest also pass independently.

## Apply

Attach `bmp-rle8-terminator.patch` and `bmp-rle8-terminator-submission.zip` to this ticket. The ZIP contains the patch, harness and this description.

From the upstream source root:

```sh
patch --dry-run -p0 < bmp-rle8-terminator.patch
patch -p0 < bmp-rle8-terminator.patch
```

Run `regression bmp-rle8-terminator` after linking regression.cpp to the built library. The harness returns 0 for the corrected behavior, 1 for a regression and 2 for usage errors. An ASan abort also indicates failure for the CUT baseline.

All patches dry-run successfully on pristine r1911. The two BMP patches also apply together. No dependency upgrades or ABI changes are needed.

The live ticket index and relevant attachments from #121/#143/#149/#154 were inspected. No equivalent patch was identified in that review; this is not an exhaustive audit of every historical discussion or attachment.
