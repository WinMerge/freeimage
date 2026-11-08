# Stage file saves and preserve the destination on encode/close failure

Status checked: 2026-11-05. No matching own ticket found; changes replacement semantics and needs platform review.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [0bd0771-preserve-destination-files-when-saving-fails.patch](0bd0771-preserve-destination-files-when-saving-fails.patch) | [0bd0771](https://github.com/danoli3/FreeImage/commit/0bd0771b0f284ec5a62fda725fd7d6257f4a00d9) | `36a3b7388eea7d93d78b7f6339b057a36fc79e44` | Dan Rosser <danoli3@gmail.com> |
| [d505799-keep-temporary-save-names-short-and-reject-special-destinations.patch](d505799-keep-temporary-save-names-short-and-reject-special-destinations.patch) | [d505799](https://github.com/danoli3/FreeImage/commit/d505799c1205409623bed9021b9f68088006da7e) | `0bd0771b0f284ec5a62fda725fd7d6257f4a00d9` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
