# Avoid the INT64 macro/typedef collision in the RAW plugin

Status checked: 2026-11-05. No matching own ticket found; check whether #195 already carries equivalent integration.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [a0bc57c-fix-int64-macro-typedef-collision-breaking-pluginraw-cpp-on-gcc-linux.patch](a0bc57c-fix-int64-macro-typedef-collision-breaking-pluginraw-cpp-on-gcc-linux.patch) | [a0bc57c](https://github.com/danoli3/FreeImage/commit/a0bc57c638f243ec068ccd90c9d79b5a03ab7df9) | `563ddd61a57af675a5fb39f6868ff2a64a32b36a` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
