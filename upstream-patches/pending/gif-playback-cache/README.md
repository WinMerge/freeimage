# Cache sequential GIF playback state and provide a build option

Status checked: 2026-11-05. No matching own ticket found. Earlier #92 also improves sequential playback; compare behavior and cache ownership.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [8395cea-fix-o-n-2-gif-playback-decoding-by-caching-sequential-playback-state.patch](8395cea-fix-o-n-2-gif-playback-decoding-by-caching-sequential-playback-state.patch) | [8395cea](https://github.com/danoli3/FreeImage/commit/8395ceabe749247c27d9be373cd2359f7ec54f20) | `b1bdbb6adddd46082e875cb98e483692d0eb8682` | Dan Rosser <danoli3@gmail.com> |
| [df001b9-gate-the-gif-playback-cache-behind-freeimage-gif-playback-cache-trim-comments.patch](df001b9-gate-the-gif-playback-cache-behind-freeimage-gif-playback-cache-trim-comments.patch) | [df001b9](https://github.com/danoli3/FreeImage/commit/df001b933cd5ee3b1edb5fa1e4c16b8ff7c5226d) | `8395ceabe749247c27d9be373cd2359f7ec54f20` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
