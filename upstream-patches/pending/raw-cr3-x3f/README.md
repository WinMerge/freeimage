# Add CR3/X3F format recognition and correct CR3 detection

Status checked: 2026-11-05. No matching own ticket found; requires the newer LibRaw integration. Retain the original contributor credits.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [16750c6-add-support-for-cr3-and-x3f-image-formats.patch](16750c6-add-support-for-cr3-and-x3f-image-formats.patch) | [16750c6](https://github.com/danoli3/FreeImage/commit/16750c6a7c2f4ff6f50e125cc8c38d341451967d) | `d1302b686cc472cb7049827546b114eff01f9265` | haobinnan <haobinnan@gmail.com> |
| [6694722-fix-incorrect-cr3-format-detection.patch](6694722-fix-incorrect-cr3-format-detection.patch) | [6694722](https://github.com/danoli3/FreeImage/commit/6694722a97c14255cafa4d97ff5657e2d76ba837) | `16750c6a7c2f4ff6f50e125cc8c38d341451967d` | haobinnan <haobinnan@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
