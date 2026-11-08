# PICT row unpacking, PackBits writes and palette bounds

Status checked: 2026-11-05. No matching own ticket found. Earlier #161 also proposes a PICT palette bound; compare before submitting.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [1824832-fix-heap-buffer-overflow-write-in-pict-unpackpictrow.patch](1824832-fix-heap-buffer-overflow-write-in-pict-unpackpictrow.patch) | [1824832](https://github.com/danoli3/FreeImage/commit/182483211b6bb06a96d43f0877cf60eb4849e892) | `606fbf3fdbdd31f9143fac48e1dd99fd5618c254` | Jorge Barredo Ferreira <jorge@barredo.info> |
| [051e9da-fix-heap-buffer-overflow-write-in-pict-unpackbits.patch](051e9da-fix-heap-buffer-overflow-write-in-pict-unpackbits.patch) | [051e9da](https://github.com/danoli3/FreeImage/commit/051e9dacbe230a8e8cb8307cf47fe3f0ea159239) | `625117b48e18a82f3f68e9be3514ae584ebfcf7b` | Michael Bommarito <michael.bommarito@gmail.com> |
| [a3eb964-fix-pict-color-table-buffer-overflow.patch](a3eb964-fix-pict-color-table-buffer-overflow.patch) | [a3eb964](https://github.com/danoli3/FreeImage/commit/a3eb9649e2a63e9e3e90737c23abbc9e0a98cda7) | `28d9bc5d7391b424638cee2ece5a98106636d04c` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
