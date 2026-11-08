# Preserve DDS DXT dimensions and decode trailing partial blocks

Applies to FreeImage SVN trunk r1911, checked 2026-10-13.

LoadDXT rounds dimensions down to multiples of four and silently crops images. Preserve the declared dimensions and restrict the full-block loops to their aligned bounds so existing trailing-block handling runs.

Original contributor: Dan Rosser; adapted from WinMerge/freeimage@6f47890. License: retain the original FreeImage file license (FIPL/GPL).
Fork fix: https://github.com/danoli3/FreeImage/commit/bf820dc

## Verification

A crafted 5x5 DXT1 image previously loads as 4x4. With the patch it loads as 5x5, including the expected red bottom-right pixel. This is distinct from upstream patch #121, which changes uncompressed LoadRGB dimensions.

The attached regression harness builds against the public FreeImage API. All six cases fail against unpatched r1911 versions of the affected files and pass after this batch. Each edited translation unit and the harness was built with `-fsanitize=address -g`, linked to the existing 3.19.21 core archive on macOS ARM64. The rest of that archive was not ASan-instrumented: this verifies the targeted code, not a full sanitized upstream build. The fork's existing MainTestSuite and VendorCodecTest also pass independently.

## Apply

Attach `dds-partial-blocks.patch` and `dds-partial-blocks-submission.zip` to this ticket. The ZIP contains the patch, harness and this description.

From the upstream source root:

```sh
patch --dry-run -p0 < dds-partial-blocks.patch
patch -p0 < dds-partial-blocks.patch
```

Run `regression dds-partial-blocks` after linking regression.cpp to the built library. The harness returns 0 for the corrected behavior, 1 for a regression and 2 for usage errors. An ASan abort also indicates failure for the CUT baseline.

All patches dry-run successfully on pristine r1911. The two BMP patches also apply together. No dependency upgrades or ABI changes are needed.

The live ticket index and relevant attachments from #121/#143/#149/#154 were inspected. No equivalent patch was identified in that review; this is not an exhaustive audit of every historical discussion or attachment.
