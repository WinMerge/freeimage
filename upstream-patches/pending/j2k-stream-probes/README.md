# Reject failed, empty or backwards JP2/J2K input stream length probes

Status checked: 2026-11-05. No matching own ticket found. This extends stream handling beyond #176; compare the J2KHelper replacement in #143.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [949cb9e-reject-invalid-jp2-and-j2k-stream-length-probes-before-decoding.patch](949cb9e-reject-invalid-jp2-and-j2k-stream-length-probes-before-decoding.patch) | [949cb9e](https://github.com/danoli3/FreeImage/commit/949cb9e6e50b93d764daca3db4745df607feca75) | `c210b8b4f290bbad9f66cb548fd2ca95532b26c0` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
