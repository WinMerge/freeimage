# Fix JPEG XR SAL annotations, missing sources, MinGW headers and non-Windows declarations

Status checked: 2026-11-05. No matching recent own ticket found. Compare #96/#115; diagnostics commit also contains tests and examples.

These are exact binary-safe diffs of committed fork changes. Each patch targets its own recorded parent commit, uses Git paths (`-p1`), and may depend on intervening fork work. They have not been rebased, applied or sanitizer-tested against SVN r1911 as part of this archive task. Preserve the original author and reporter credits when preparing a submission.

| Patch | Fork commit | Parent / patch base | Original author |
| --- | --- | --- | --- |
| [fad7934-compile-jpeg-xr-on-windows-and-fix-the-msvc-sal-break.patch](fad7934-compile-jpeg-xr-on-windows-and-fix-the-msvc-sal-break.patch) | [fad7934](https://github.com/danoli3/FreeImage/commit/fad7934b3de35fdabda3906e55331cd8e1ae82a1) | `f7d3cc9825e3f4530a76e3281e1b27f99a02ba71` | Dan Rosser <danoli3@gmail.com> |
| [e8003f9-link-the-rest-of-jxrlib-and-stop-shadowing-mingw-guiddef-h.patch](e8003f9-link-the-rest-of-jxrlib-and-stop-shadowing-mingw-guiddef-h.patch) | [e8003f9](https://github.com/danoli3/FreeImage/commit/e8003f9fd3c3c8fa8fd4d3a9eafdb5cffd896732) | `76211a66deccdadc926826dce8d23f6737a44b16` | Dan Rosser <danoli3@gmail.com> |
| [cde9a94-use-mingw-s-byteswap-ulong-instead-of-a-conflicting-local-one.patch](cde9a94-use-mingw-s-byteswap-ulong-instead-of-a-conflicting-local-one.patch) | [cde9a94](https://github.com/danoli3/FreeImage/commit/cde9a9493fb45f3c8ed9b4e905690e20947c6722) | `e8003f9fd3c3c8fa8fd4d3a9eafdb5cffd896732` | Dan Rosser <danoli3@gmail.com> |
| [30ed59f-add-jp2-jxr-loading-and-display-diagnostics-with-sample-checks.patch](30ed59f-add-jp2-jxr-loading-and-display-diagnostics-with-sample-checks.patch) | [30ed59f](https://github.com/danoli3/FreeImage/commit/30ed59fc3665eb15758d642e1ae7fd1356d4224e) | `85b7994ec03f99a2e8999fa6d77dc72986e4c07e` | Dan Rosser <danoli3@gmail.com> |

Inspect each commit and its changed tests for the original validation context. This catalog records provenance and patch integrity; it does not repeat the original bug validation.
