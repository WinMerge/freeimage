# Avoid aligned vector stack spills in bundled codecs with MinGW GCC

Status checked: 2026-11-05. No matching own ticket found; applies to the fork bundled-codec CMake integration.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [903e9c7-emit-unaligned-vector-moves-for-mingw-gcc-211.patch](903e9c7-emit-unaligned-vector-moves-for-mingw-gcc-211.patch) | [903e9c7](https://github.com/danoli3/FreeImage/commit/903e9c772ef02ea7c741a9c7dab086c15f99b661) | `2aaa8b85527559661e7a5046dbd1203636eb35c1` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
