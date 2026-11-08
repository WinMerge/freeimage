# Reject signed BMP RLE cursor wrap before a subsequent write (GHSA-7pp5-4w3w-437c)

Status checked: 2026-11-05. No matching own ticket found; separate from #188/#189 and RLE4 allocation fixes.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [f89723b-reject-wrapped-bmp-rle-cursors-before-they-write.patch](f89723b-reject-wrapped-bmp-rle-cursors-before-they-write.patch) | [f89723b](https://github.com/danoli3/FreeImage/commit/f89723b04846f97e996e9aed96216a9e171bfafa) | `5813b07b344e310f621f98b59208dc9c3e683b58` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
