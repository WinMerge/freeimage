# Update LibRaw 0.22.2 to 0.22.3 with decoder hardening

Status checked: 2026-11-05. Later than submitted #195 (0.22.2); prepare an SVN follow-up retaining integration adaptations.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [a13c585-sync-bundled-libraw-0-22-2-0-22-3.patch](a13c585-sync-bundled-libraw-0-22-2-0-22-3.patch) | [a13c585](https://github.com/danoli3/FreeImage/commit/a13c58574f31358fa8eb27183f2ece05a4c759bf) | `6f24892f9dd4e0584b37ff136ec443373f315ffe` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
