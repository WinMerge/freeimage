# Overflow-safe BMP RLE4 pixel counts and 32-bit allocation arithmetic (CVE-2023-47994)

Status checked: 2026-11-05. No matching own ticket found; separate from BMP compatibility submissions #188/#189.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [a8c20af-pluginbmp-size-t-arithmetic-in-loadpixeldatarle4-cve-2023-47994.patch](a8c20af-pluginbmp-size-t-arithmetic-in-loadpixeldatarle4-cve-2023-47994.patch) | [a8c20af](https://github.com/danoli3/FreeImage/commit/a8c20af92e9b7546e780037955d1042421d47c02) | `8fea9cd34bf941143eda2f82dc05129a02957985` | w4ffl35 <25737761+w4ffl35@users.noreply.github.com> |
| [5be4529-pluginbmp-bound-the-rle4-pixel-count-for-32-bit-size-t.patch](5be4529-pluginbmp-bound-the-rle4-pixel-count-for-32-bit-size-t.patch) | [5be4529](https://github.com/danoli3/FreeImage/commit/5be4529e696c948ea938d85f9dd3bd83275345b8) | `6576846519ad9cb444827a18c9dec6e9a3407989` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
