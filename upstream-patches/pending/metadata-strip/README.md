# Strip auxiliary metadata on load/save or existing images; allow compiling the feature out

Status checked: 2026-11-05. No matching own ticket found; public flags/API and codec integration need review.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [d6f1f93-add-metadata-stripping-for-image-load-and-save-239.patch](d6f1f93-add-metadata-stripping-for-image-load-and-save-239.patch) | [d6f1f93](https://github.com/danoli3/FreeImage/commit/d6f1f935f210f54cec3f31dc85d49dafebbe1bd6) | `113295a6ca8735ac05fb56bd99c543b46763af73` | Dan Rosser <danoli3@gmail.com> |
| [3064322-allow-metadata-stripping-to-be-compiled-out.patch](3064322-allow-metadata-stripping-to-be-compiled-out.patch) | [3064322](https://github.com/danoli3/FreeImage/commit/30643220480d98d6c570e2a94ac2de5feb836671) | `d6f1f935f210f54cec3f31dc85d49dafebbe1bd6` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
