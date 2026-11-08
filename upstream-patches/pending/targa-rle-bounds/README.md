# Bound TARGA RLE packets by image rows; handle unaligned and short cache reads

Status checked: 2026-11-05. Follow-up missing from our #177 attachment; that earlier patch only widens the minimum cache.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [2124a26-plugintarga-bound-rle-decode-by-image-rows-cve-2024-28581.patch](2124a26-plugintarga-bound-rle-decode-by-image-rows-cve-2024-28581.patch) | [2124a26](https://github.com/danoli3/FreeImage/commit/2124a26aceab5d124ec6728672d216f8a1eb1019) | `8fea9cd34bf941143eda2f82dc05129a02957985` | w4ffl35 <25737761+w4ffl35@users.noreply.github.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
