# Use thread-local buffers for FreeImage_TagToString formatting

Status checked: 2026-11-05. No matching own ticket found; review supported compiler/thread-local requirements.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [735872d-make-freeimage-tagtostring-thread-safe.patch](735872d-make-freeimage-tagtostring-thread-safe.patch) | [735872d](https://github.com/danoli3/FreeImage/commit/735872d76a640841838533adfd3b1db229b17c84) | `c0e14f4dee18b74f83855fe3cd40a65888998b8e` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
