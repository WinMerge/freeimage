# Optional HEIF/AVIF still-image decoding/encoding, EXIF/ICC and thumbnails

Status checked: 2026-11-05. No matching own ticket found. Earlier #137 proposes a different AVIF plugin; backend/build integration is required.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [cce5cad-add-optional-heif-and-avif-decoding-with-system-libheif.patch](cce5cad-add-optional-heif-and-avif-decoding-with-system-libheif.patch) | [cce5cad](https://github.com/danoli3/FreeImage/commit/cce5cad9a81bf865b86ba495c7329f24acf27e71) | `546cf69fb061bf1cdcfcdd5c61f5853cbfe5f6cc` | Dan Rosser <danoli3@gmail.com> |
| [360c5f5-add-heif-and-avif-encoding-exif-and-thumbnail-import.patch](360c5f5-add-heif-and-avif-encoding-exif-and-thumbnail-import.patch) | [360c5f5](https://github.com/danoli3/FreeImage/commit/360c5f530af2a8b4e3ae07ffe8f5052e66ea624b) | `1b476e184fd8b788f97186db6c19a93c95c4d02d` | Dan Rosser <danoli3@gmail.com> |
| [8d2f56f-preserve-heif-primary-images-when-optional-thumbnails-fail.patch](8d2f56f-preserve-heif-primary-images-when-optional-thumbnails-fail.patch) | [8d2f56f](https://github.com/danoli3/FreeImage/commit/8d2f56f22ec9e4ef8abcc4c814a4e90f2a4679f9) | `42a8566f679f82a09cbb4f103117fce54e33a18e` | Dan Rosser <danoli3@gmail.com> |
| [dd0e629-allow-heif-avif-saving-to-be-compiled-out.patch](dd0e629-allow-heif-avif-saving-to-be-compiled-out.patch) | [dd0e629](https://github.com/danoli3/FreeImage/commit/dd0e629a6065d07a9c06a4d7ede7a30fdda09ae5) | `54e2e21f53cb6c316363a0d1e5063e4261434966` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
