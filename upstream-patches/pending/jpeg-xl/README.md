# Optional JPEG XL still-image decoding/encoding and read-only build switch

Status checked: 2026-11-05. No matching own ticket found; depends on libjxl and separate build integration.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [a376d9b-add-optional-jpeg-xl-still-image-decoding-and-encoding.patch](a376d9b-add-optional-jpeg-xl-still-image-decoding-and-encoding.patch) | [a376d9b](https://github.com/danoli3/FreeImage/commit/a376d9b9bc22c6ba572d1c0b9d6b2cee56934962) | `360c5f530af2a8b4e3ae07ffe8f5052e66ea624b` | Dan Rosser <danoli3@gmail.com> |
| [7d6ee93-allow-jpeg-xl-saving-to-be-compiled-out.patch](7d6ee93-allow-jpeg-xl-saving-to-be-compiled-out.patch) | [7d6ee93](https://github.com/danoli3/FreeImage/commit/7d6ee93680b5f563b12594aa0eece9e2922087b1) | `dd0e629a6065d07a9c06a4d7ede7a30fdda09ae5` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
