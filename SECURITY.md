# Security policy

## Reporting a vulnerability

Please report security problems privately, not in a public issue or pull
request.

Use GitHub's private vulnerability reporting: open the repository's
**Security** tab and choose **Report a vulnerability**, or go to
<https://github.com/danoli3/FreeImage/security/advisories/new>. Only the
maintainers can see the report, and the fix can be prepared in a private
advisory before anything is published.

Helpful details:

- The FreeImage version (`FreeImage_GetVersion()`) or commit you tested.
- The image format and API call, for example `FreeImage_Load(FIF_TIFF, ...)`
  or `FreeImage_LoadFromMemory`.
- A small input file that reproduces the problem. Attach it to the report
  rather than linking to a public location.
- Sanitizer output (for example AddressSanitizer) or a backtrace, and the
  platform and compiler.
- Whether you plan to request a CVE yourself, and how you would like to be
  credited.

## Supported versions

Fixes go into the next 3.19.x release from `master`. Only the latest 3.19.x
release is supported; please check that the problem still reproduces there or
on `master`. Upstream FreeImage 3.18 and earlier SourceForge releases are not
maintained here.

## Scope

In scope:

- FreeImage's own code under `Source/`, including the format plugins,
  metadata parsers and toolkit functions.
- The bundled third-party libraries as they are built and used by FreeImage.
  If the bug is in the library itself, please also report it upstream. We
  update or patch the bundled copy here.

Out of scope:

- Bugs that only occur in system-installed libraries selected with
  `FREEIMAGE_USE_SYSTEM_LIBS` or `USE_SYSTEM_*`. Report those to the library
  or your distribution.
- Excessive memory use or slow decoding of very large but valid images,
  unless a small input causes it.

## After a report

We confirm the problem, prepare a fix with a regression test, and release it
in a new 3.19.x version. The reporter is credited in the advisory and the
changelog unless they ask not to be. CVE identifiers are named in
`CHANGELOG.md` and the release notes once assigned.
