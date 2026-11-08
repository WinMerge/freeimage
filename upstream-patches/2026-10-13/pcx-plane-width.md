# Reject three-plane PCX images with insufficient bytes per plane

Applies to FreeImage SVN trunk r1911, checked 2026-10-13.

For 8-bit, three-plane PCX, the expansion loop reads width bytes per plane while the file supplies bytes_per_line bytes. Reject width > bytes_per_line before expansion. The existing over-allocation can hide a physical out-of-bounds read while still exposing uninitialized data.

Original contributor: wangshuo. License: retain the original FreeImage file license (FIPL/GPL).
Fork fix: https://github.com/danoli3/FreeImage/commit/a4473a4

## Verification

The malformed four-pixel/two-byte-per-plane image loads before this fix and is rejected after it; a correctly sized two-pixel control still loads. ASan is not an uninitialized-memory detector.

The attached regression harness builds against the public FreeImage API. All six cases fail against unpatched r1911 versions of the affected files and pass after this batch. Each edited translation unit and the harness was built with `-fsanitize=address -g`, linked to the existing 3.19.21 core archive on macOS ARM64. The rest of that archive was not ASan-instrumented: this verifies the targeted code, not a full sanitized upstream build. The fork's existing MainTestSuite and VendorCodecTest also pass independently.

## Apply

Attach `pcx-plane-width.patch` and `pcx-plane-width-submission.zip` to this ticket. The ZIP contains the patch, harness and this description.

From the upstream source root:

```sh
patch --dry-run -p0 < pcx-plane-width.patch
patch -p0 < pcx-plane-width.patch
```

Run `regression pcx-plane-width` after linking regression.cpp to the built library. The harness returns 0 for the corrected behavior, 1 for a regression and 2 for usage errors. An ASan abort also indicates failure for the CUT baseline.

All patches dry-run successfully on pristine r1911. The two BMP patches also apply together. No dependency upgrades or ABI changes are needed.

The live ticket index and relevant attachments from #121/#143/#149/#154 were inspected. No equivalent patch was identified in that review; this is not an exhaustive audit of every historical discussion or attachment.
