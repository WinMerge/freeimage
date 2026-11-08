# Unicode single-image fixes and Windows Unicode multipage/cache paths

Status checked: 2026-11-05. No matching recent own ticket found; compare historical #96/#115 before resubmitting older hunks.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [2058ef8-unicode-wchar-fixes-for-multilanguages.patch](2058ef8-unicode-wchar-fixes-for-multilanguages.patch) | [2058ef8](https://github.com/danoli3/FreeImage/commit/2058ef89f0455e23f993a8e5548d6fa8d9df0e15) | `e803a5868bf276a29d85858f7dd1b5f18e626952` | Dan Rosser <danoli@gmail.com> |
| [a03ea3d-load-wchar-path.patch](a03ea3d-load-wchar-path.patch) | [a03ea3d](https://github.com/danoli3/FreeImage/commit/a03ea3dd260e4549187b8e4c27f92414da1cbfb4) | `b86fa1dcf1ab6eb4cdfb1c08a342c85b546c0b20` | Dan Rosser <danoli3@gmail.com> |
| [bcec225-add-windows-unicode-paths-for-multipage-images.patch](bcec225-add-windows-unicode-paths-for-multipage-images.patch) | [bcec225](https://github.com/danoli3/FreeImage/commit/bcec225ee09c74d9388ba4999e7df951315696ee) | `cc2bfc1b67d3a10e2c203c56932b09ffb52a4477` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
