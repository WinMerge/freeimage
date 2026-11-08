# Unbundle dependencies and repair PNG/zlib overrides, system OpenJPEG imports and Imath includes

Status checked: 2026-11-05. No matching own ticket found; integration spans multiple codecs and newer vendor APIs.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [10c850e-unbundle-patch.patch](10c850e-unbundle-patch.patch) | [10c850e](https://github.com/danoli3/FreeImage/commit/10c850e176316527a2c8c533a907e76db7d3762e) | `95a189964969bf69b8e1c39577883b8b8ca6db6c` | Dan Rosser <danoli3@gmail.com> |
| [128b715-keep-build-libpng-off-build-zlib-off-png-library-zlib-library-working.patch](128b715-keep-build-libpng-off-build-zlib-off-png-library-zlib-library-working.patch) | [128b715](https://github.com/danoli3/FreeImage/commit/128b7157c304c5a4600cb6f2db1d319e4e4b96d1) | `10c850e176316527a2c8c533a907e76db7d3762e` | Dan Rosser <danoli3@gmail.com> |
| [e33c25c-fix-cmake-configure-crash-when-png-library-is-set-without-zlib-library.patch](e33c25c-fix-cmake-configure-crash-when-png-library-is-set-without-zlib-library.patch) | [e33c25c](https://github.com/danoli3/FreeImage/commit/e33c25c9a3a70e58ee6f3f23219e05454c028ba3) | `353fcb93b2a654c8b29a3c50dfb71dc8353c23ca` | Dan Rosser <danoli3@gmail.com> |
| [627ab4e-use-shared-openjpeg-imports-in-system-library-packages.patch](627ab4e-use-shared-openjpeg-imports-in-system-library-packages.patch) | [627ab4e](https://github.com/danoli3/FreeImage/commit/627ab4e0d866724f69e7fcfd533b5bd34269f290) | `f4cf8558d9b4795758f1c3c6df777096f9afae92` | Dan Rosser <danoli3@gmail.com> |
| [5a6ab5d-avoid-bundled-imath-headers-in-system-openexr-builds.patch](5a6ab5d-avoid-bundled-imath-headers-in-system-openexr-builds.patch) | [5a6ab5d](https://github.com/danoli3/FreeImage/commit/5a6ab5d6c72af3a1f555b907aeab0c23b840cedf) | `8747592e101b323ae02285a0c0736a69ca529672` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
