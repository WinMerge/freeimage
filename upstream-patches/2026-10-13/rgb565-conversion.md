# Select RGB565 conversion using the distinguishing green mask

Applies to FreeImage SVN trunk r1911, checked 2026-10-13.

Select the 565 conversion path from the green mask, consistent with other conversions, when producers do not supply the expected red mask. The blue mask is identical for RGB555 and RGB565.

Original contributor: Dan Rosser; ported from WinMerge/freeimage@115b7d5. License: retain the original FreeImage file license (FIPL/GPL).
Fork fix: https://github.com/danoli3/FreeImage/commit/1a9ccb5

## Verification

A 16-bit input with the 565 green mask and a missing red mask produces incorrect red/green components before the fix and a pure green pixel after conversion with the fix.

The attached regression harness builds against the public FreeImage API. All six cases fail against unpatched r1911 versions of the affected files and pass after this batch. Each edited translation unit and the harness was built with `-fsanitize=address -g`, linked to the existing 3.19.21 core archive on macOS ARM64. The rest of that archive was not ASan-instrumented: this verifies the targeted code, not a full sanitized upstream build. The fork's existing MainTestSuite and VendorCodecTest also pass independently.

## Apply

Attach `rgb565-conversion.patch` and `rgb565-conversion-submission.zip` to this ticket. The ZIP contains the patch, harness and this description.

From the upstream source root:

```sh
patch --dry-run -p0 < rgb565-conversion.patch
patch -p0 < rgb565-conversion.patch
```

Run `regression rgb565-conversion` after linking regression.cpp to the built library. The harness returns 0 for the corrected behavior, 1 for a regression and 2 for usage errors. An ASan abort also indicates failure for the CUT baseline.

All patches dry-run successfully on pristine r1911. The two BMP patches also apply together. No dependency upgrades or ABI changes are needed.

The live ticket index and relevant attachments from #121/#143/#149/#154 were inspected. No equivalent patch was identified in that review; this is not an exhaustive audit of every historical discussion or attachment.
