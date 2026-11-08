# Accept BMP information-header sizes already handled by the loader

Applies to FreeImage SVN trunk r1911, checked 2026-10-13.

The header checker rejected supported larger information headers even though Load dispatches them. Accept sizes 40, 52, 56, 64, 108 and 124; continue rejecting unknown sizes.

Original contributor: Thomas Beutlich. License: retain the original FreeImage file license (FIPL/GPL).
Fork fix: https://github.com/danoli3/FreeImage/commit/7646cfe

## Verification

Regression checks load one-pixel Windows BMPs with sizes 40/52/56/108/124. The OS/2 size 64 acceptance is reviewed in the patch but is not exercised by this fixture. This is separate from upstream #154, whose patch addresses a pixel source read.

The attached regression harness builds against the public FreeImage API. All six cases fail against unpatched r1911 versions of the affected files and pass after this batch. Each edited translation unit and the harness was built with `-fsanitize=address -g`, linked to the existing 3.19.21 core archive on macOS ARM64. The rest of that archive was not ASan-instrumented: this verifies the targeted code, not a full sanitized upstream build. The fork's existing MainTestSuite and VendorCodecTest also pass independently.

## Apply

Attach `bmp-extended-headers.patch` and `bmp-extended-headers-submission.zip` to this ticket. The ZIP contains the patch, harness and this description.

From the upstream source root:

```sh
patch --dry-run -p0 < bmp-extended-headers.patch
patch -p0 < bmp-extended-headers.patch
```

Run `regression bmp-extended-headers` after linking regression.cpp to the built library. The harness returns 0 for the corrected behavior, 1 for a regression and 2 for usage errors. An ASan abort also indicates failure for the CUT baseline.

All patches dry-run successfully on pristine r1911. The two BMP patches also apply together. No dependency upgrades or ABI changes are needed.

The live ticket index and relevant attachments from #121/#143/#149/#154 were inspected. No equivalent patch was identified in that review; this is not an exhaustive audit of every historical discussion or attachment.
