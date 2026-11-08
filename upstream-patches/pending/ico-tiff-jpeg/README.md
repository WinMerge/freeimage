# Reject oversized ICOs and handle TIFF/JPEG sampling-factor mismatches safely

Status checked: 2026-11-05. No matching own ticket found. ICO checks extend #170; TIFF changes require the matching libtiff baseline.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [23f4c9a-fix-loading-of-oversized-icos-and-tiffs-with-mismatched-jpeg-sampling-factors.patch](23f4c9a-fix-loading-of-oversized-icos-and-tiffs-with-mismatched-jpeg-sampling-factors.patch) | [23f4c9a](https://github.com/danoli3/FreeImage/commit/23f4c9ac9ee4dc877dce3dc382c6611770ea021d) | `9942499f477154a81ffe1358de47f0d54cb35b4b` | haobinnan <haobinnan@gmail.com> |
| [4c7b7cb-handle-tiff-jpeg-sampling-mismatches-safely.patch](4c7b7cb-handle-tiff-jpeg-sampling-mismatches-safely.patch) | [4c7b7cb](https://github.com/danoli3/FreeImage/commit/4c7b7cbbb14282d10721038f48b0c2c261eb5586) | `23f4c9ac9ee4dc877dce3dc382c6611770ea021d` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
