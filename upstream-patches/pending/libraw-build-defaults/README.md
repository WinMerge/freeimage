# Build bundled LibRaw by default and avoid dllimport on bundled sources

Status checked: 2026-11-05. No matching own ticket found; compare build adaptations already bundled into #195.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [ad71150-build-libraw-by-default.patch](ad71150-build-libraw-by-default.patch) | [ad71150](https://github.com/danoli3/FreeImage/commit/ad711509b600a0a43cccf7d6a134ac3b39698a28) | `c37c3ba3b50c1135be60b21b52efd9e70d0f7f47` | Dan Rosser <danoli3@gmail.com> |
| [855fe4e-compile-bundled-libraw-without-dllimport.patch](855fe4e-compile-bundled-libraw-without-dllimport.patch) | [855fe4e](https://github.com/danoli3/FreeImage/commit/855fe4e1a42820002cce23ec0647bb09dd9e5ecc) | `ad711509b600a0a43cccf7d6a134ac3b39698a28` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
