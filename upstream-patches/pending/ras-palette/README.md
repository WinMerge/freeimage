# Return safely when a RAS palette allocation is unavailable

Status checked: 2026-11-05. Earlier #163 already submits a palette-pointer check from another contributor; do not claim this is new without comparison.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [8445931-fix-ras-crash-on-null-palette.patch](8445931-fix-ras-crash-on-null-palette.patch) | [8445931](https://github.com/danoli3/FreeImage/commit/8445931ae8d69d76940848f445f1a1ddbe239e27) | `a3eb9649e2a63e9e3e90737c23abbc9e0a98cda7` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
