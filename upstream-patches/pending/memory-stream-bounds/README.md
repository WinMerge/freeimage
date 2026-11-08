# Prevent read/write byte-count wrap and write-end overflow near INT_MAX

Status checked: 2026-11-05. No matching own ticket found. Earlier #149 concerns seek-position narrowing, not these size/count and write-end changes.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [7f93bf8-freeimageio-width-safe-memoryreadproc-total-3-19-regression-of-cve-2023-47992-class.patch](7f93bf8-freeimageio-width-safe-memoryreadproc-total-3-19-regression-of-cve-2023-47992-class.patch) | [7f93bf8](https://github.com/danoli3/FreeImage/commit/7f93bf8b0b1ff27c77d5f487443c9f9ec63c5754) | `8fea9cd34bf941143eda2f82dc05129a02957985` | w4ffl35 <25737761+w4ffl35@users.noreply.github.com> |
| [23ea250-freeimageio-overflow-safe-memorywriteproc-total-same-bug-class.patch](23ea250-freeimageio-overflow-safe-memorywriteproc-total-same-bug-class.patch) | [23ea250](https://github.com/danoli3/FreeImage/commit/23ea250de4d7deab70fb370acb33aff577a8c79c) | `7f93bf8b0b1ff27c77d5f487443c9f9ec63c5754` | w4ffl35 <25737761+w4ffl35@users.noreply.github.com> |
| [d1439d0-freeimageio-bound-memorywriteproc-end-position-before-adding.patch](d1439d0-freeimageio-bound-memorywriteproc-end-position-before-adding.patch) | [d1439d0](https://github.com/danoli3/FreeImage/commit/d1439d0d2dc0ff21eb00e6adf90f6ab35fee6b51) | `9793cc71cf8a3d60a1bfab5a18c9274e7a79f20d` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
