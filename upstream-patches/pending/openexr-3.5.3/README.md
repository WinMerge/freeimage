# Update OpenEXR 3.5.2 to 3.5.3 for malformed-file and codec fixes

Status checked: 2026-11-05. Later than submitted #197 (3.5.2); requires that vendor series and adaptation to SVN build files.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [90bbeca-update-bundled-openexr-to-3-5-3.patch](90bbeca-update-bundled-openexr-to-3-5-3.patch) | [90bbeca](https://github.com/danoli3/FreeImage/commit/90bbecac2ed5213ad688c170dcd55b5c130f6d8e) | `7096f98c984d09950e96f4b5bd03478b80f20174` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
