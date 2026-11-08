# Expose linked dependency versions and bundled/system provenance

Status checked: 2026-11-05. No matching own ticket found; public API feature.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [4f391e6-expose-linked-dependency-versions-and-provenance.patch](4f391e6-expose-linked-dependency-versions-and-provenance.patch) | [4f391e6](https://github.com/danoli3/FreeImage/commit/4f391e61dbb30f4ac82e8756e882059c7f8f5161) | `a376d9b9bc22c6ba572d1c0b9d6b2cee56934962` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
