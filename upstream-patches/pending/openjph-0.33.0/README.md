# Update OpenJPH 0.32.0 to 0.33.0, including one-row allocation and NLT checks

Status checked: 2026-11-05. Later than submitted #197; requires the corresponding OpenEXR/OpenJPH source baseline.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [8dd7c48-update-bundled-openjph-to-0-33-0.patch](8dd7c48-update-bundled-openjph-to-0-33-0.patch) | [8dd7c48](https://github.com/danoli3/FreeImage/commit/8dd7c489763a06ca3d65410213eae7b2f2f6b3e5) | `3d556aa275df35763dcb155b8969fb3e08c9817e` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
