# Add separate 64-bit TIFF input callbacks and APIs while preserving existing plugin ABI

Status checked: 2026-11-05. No matching own ticket found; public API feature, requires ABI and large-file validation.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [c0753bb-add-64-bit-tiff-input-interface.patch](c0753bb-add-64-bit-tiff-input-interface.patch) | [c0753bb](https://github.com/danoli3/FreeImage/commit/c0753bb45f432b76ed6d42401974006b6ba6d612) | `9a22df43d65a74e1f1a6294c95792c1190fbe826` | Dan Rosser <danoli3@gmail.com> |
| [6868bc4-preserve-plugin-abi-and-harden-64-bit-tiff-input.patch](6868bc4-preserve-plugin-abi-and-harden-64-bit-tiff-input.patch) | [6868bc4](https://github.com/danoli3/FreeImage/commit/6868bc4c7726f0e28482c02b307461eadf7da8da) | `c0753bb45f432b76ed6d42401974006b6ba6d612` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
