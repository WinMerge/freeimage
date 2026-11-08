# Publish consumer includes/static defines, Windows DLL exports, install components and shared versions

Status checked: 2026-11-05. No matching own ticket found; compare independent CMake proposal #141.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [ba2d5be-expose-freeimage-h-and-freeimage-lib-to-add-subdirectory-consumers.patch](ba2d5be-expose-freeimage-h-and-freeimage-lib-to-add-subdirectory-consumers.patch) | [ba2d5be](https://github.com/danoli3/FreeImage/commit/ba2d5be063cd6b398f8298cc00f4bedd7fc5a837) | `44f71d82b9d552fcf0a80dfc9d566053fd249eeb` | Dan Rosser <danoli3@gmail.com> |
| [8741e50-split-the-cmake-install-into-runtime-and-development-components.patch](8741e50-split-the-cmake-install-into-runtime-and-development-components.patch) | [8741e50](https://github.com/danoli3/FreeImage/commit/8741e50e97b03a598cd425fa8c99ed397350bb00) | `ba2d5be063cd6b398f8298cc00f4bedd7fc5a837` | Dan Rosser <danoli3@gmail.com> |
| [8e67a64-define-freeimage-exports-when-building-the-windows-dll.patch](8e67a64-define-freeimage-exports-when-building-the-windows-dll.patch) | [8e67a64](https://github.com/danoli3/FreeImage/commit/8e67a64e618b3e56826b3d8a690b83d1f2d037f7) | `8741e50e97b03a598cd425fa8c99ed397350bb00` | Dan Rosser <danoli3@gmail.com> |
| [b4531e4-set-version-properties-for-freeimage-target.patch](b4531e4-set-version-properties-for-freeimage-target.patch) | [b4531e4](https://github.com/danoli3/FreeImage/commit/b4531e4c823b9a9148a1c7104bd37778b8665cb5) | `5af27d055ad31fbf99631a55a5de19b3820a4790` | Oliver Old <mail@oold.dev> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
