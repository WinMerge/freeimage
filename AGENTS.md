# AGENTS.md

Notes for AI agents (and humans) working on this repo. This is a fork of
FreeImage 3 with an active CMake-based build, security-patch backlog, and CI
matrix - the points below are things that took real investigation to learn
and aren't obvious from the code alone.

## Build system

- **CMake is primary.** `CMakeLists.txt` builds FreeImage plus every vendored
  sub-dependency from one invocation. See `README.md`'s "Platform-specific
  examples" section for copy-pasteable commands per toolchain.
- **`Makefile`/`Makefile.gnu`/`Makefile.srcs` is a separate, hand-maintained
  build path** with its own source list. It silently drifts out of sync with
  CMake's source lists (see issue #33) - there's no automatic way to keep
  them in sync, only the `make-build` CI job as a tripwire. If you add/remove
  a source file in `CMakeLists.txt`, check whether `Makefile.srcs` needs the
  same change.
- **`CMakePresets.json`** provides `vs2022`, `vs2026` (Visual Studio
  generator, Windows-only via `condition`) and `ninja-msvc` (Ninja + real
  `cl.exe`, needs a Developer Command Prompt / `vcvarsall.bat` environment
  already active). All three are only testable on a real Windows CI runner -
  the VS generators don't exist in CMake on macOS/Linux at all.
- **Versioning is git-tag-driven, and the hand-maintained copies must
  move in the same release PR.** `git describe` feeds CMake. Several
  other copies are not derived from the tag. The full list, and which
  commit in the 3.19.17 PR is the bump, is under
  [Cutting a release](#cutting-a-release).
- **Sub-dependency unbundling**: `FREEIMAGE_USE_SYSTEM_LIBS` (master switch)
  and per-library `USE_SYSTEM_*` options let a consumer link system
  zlib/libpng/libtiff/libjpeg/openjpeg/webp/libraw/OpenEXR/jxrlib instead of
  the bundled copies. Useful both for smaller builds and for dodging CVEs in
  old bundled library versions (see issue #35 below).

## Cutting a release

Do this as one commit on a branch from current `master`, after the
fixes for that release are already merged. Open the PR, merge it, then
tag **that merge commit**. Do not tag the branch tip before the PR
lands, and do not tag an earlier commit that only bumped the header.

PR [#129](https://github.com/danoli3/FreeImage/pull/129)
(`release/3.19.17`) is the pattern. The version bump is the commit
titled `FreeImage 3.19.17`. A later commit on the same PR only
rewrote the iOS build steps in `README.iphone` and is not part of the
version change.

The 3.19.16 tag was cut when only `FREEIMAGE_RELEASE_SERIAL` had
moved (PR #122). The CMake fallback and the checked-in `.rc` files
were still 3.19.12 until a later commit, so a source archive with no
`.git` directory and the legacy Visual Studio projects advertised the
old release. Do not repeat that split.

### What reports the version

`FreeImage_GetVersion()` prints `FREEIMAGE_MAJOR_VERSION`,
`FREEIMAGE_MINOR_VERSION`, and `FREEIMAGE_RELEASE_SERIAL` from
`Source/FreeImage.h`. That header is the public C contract and is
**not** filled in from the git tag, because the header has to be right
when the tree is vendored without this repo's CMake.

A CMake configure runs `git describe --tags --long --match
"[0-9]*.[0-9]*.[0-9]*"` before `project()`. On the tag, the project
version is exactly `3.19.17`. Between tags it is
`3.19.17+<commits>.<hash>`. `FreeImage.rc.in` and
`Wrapper/FreeImagePlus/FreeImagePlus.rc.in` are generated from that
string. Do not hand-edit the `.rc.in` templates to bump a release.

Until the new tag exists, a checkout that still has `.git` keeps
describing the **previous** tag plus a commit count, even after
`FreeImage_GetVersion()` already returns the new serial. The fallback
below is what a tarball with no `.git` directory uses.

### Files to change in the version commit

| What the consumer sees | File | Edit |
| --- | --- | --- |
| `FreeImage_GetVersion()` | `Source/FreeImage.h` | `FREEIMAGE_RELEASE_SERIAL`. Major and minor only when those actually change. |
| CMake version with no git history | `CMakeLists.txt` | `FREEIMAGE_VERSION_FALLBACK`, and the `3.19.x` examples in the comments directly above and below it. |
| Legacy Visual Studio projects, which do not run the `.rc.in` step | `FreeImage.rc` and `Wrapper/FreeImagePlus/FreeImagePlus.rc` | `FILEVERSION`, `PRODUCTVERSION`, `FileVersion`, and `ProductVersion` (`3,19,17,0` and `"3, 19, 17, 0"`). Both files are CRLF. |
| The version sentence in the project README | `README.md` | The "This tree is FreeImage …" line and the sentence that names the no-git fallback. |
| Vendored-library checklist | `THIRD_PARTY_VERSIONS.md` | Recheck the vendored column against the headers under `Source/` in the same step. The file says to do this with the serial bump. |

In PR #129 those paths are exactly the version commit, plus the
platform README lead-ins (`README.linux`, `README.osx`,
`README.minGW`, `README.solaris`, `README.iphone`) that were refreshed
in that same commit. `README.iphone`'s iOS CMake commands were a
follow-up commit.

### Leave these alone

- `FreeImage.rc.in` and `FreeImagePlus.rc.in`. CMake fills them.
- The old VB6 `.bas` and Delphi `.pas` wrappers. They still declare
  release serial 0 and were not moved for 3.19.12 or 3.19.16.
- `git describe` examples that are not the fallback constant. Only the
  examples that quote the current release number, next to
  `FREEIMAGE_VERSION_FALLBACK`.

### After the PR merges

```bash
git checkout master && git pull
git tag 3.19.17
git push origin 3.19.17
```

The tag name is `3.19.17`, with no `v` prefix. The describe match is
`[0-9]*.[0-9]*.[0-9]*`. An old tag `3.19.21` points at the same 2024
commit as `3.19.3`. Describe still prefers the closer ancestor, which
is the newer `3.19.x` tag, so do not delete `3.19.21` as part of a
routine bump.

Confirm on the tagged commit:

- `FreeImage_GetVersion()` is `3.19.17`
- `git describe --tags --match '[0-9]*.[0-9]*.[0-9]*'` prints `3.19.17`.
  With `--long` the same commit is `3.19.17-0-g<hash>`, and CMake treats
  the `-0-` as exactly on the tag.
- `FREEIMAGE_VERSION_FALLBACK` and both checked-in `.rc` files say
  3.19.17

## CI (`.github/workflows/cmake.yml`)

- **`push:` is scoped to `branches: [master]`.** An unscoped `push:` trigger
  fires for every push to an open PR's branch *in addition to* the
  `pull_request:` trigger for the same commit - doubling every CI run. Don't
  remove that scoping without re-introducing that problem.
- **`pull_request:` builds the merge commit** (PR branch merged into current
  `master`), not just the raw branch tip - this is the more useful of the two
  signals and has caught real bugs that only appeared once merged with
  master (see the OpenEXR/mingw item below).
- **Windows jobs needing a VS toolchain must set `shell: pwsh`**, not the
  workflow's default `bash`. Git Bash can't represent env var names
  containing parentheses (e.g. `ProgramFiles(x86)`), which CMake's VS
  Installer instance-detection reads - this manifests as `could not find any
  instance of Visual Studio` even when it's genuinely installed. Affects the
  VS generator, `ninja-msvc` preset, and `ilammy/msvc-dev-cmd`-based jobs.
- **clang-cl needs target-feature workarounds for bundled SIMD code.** MSVC
  doesn't gate SSE/AVX intrinsic headers on `/arch:`, but clang-cl does, and
  several vendored libraries (LibWebP's `cpu.c`, OpenEXR's `ImfZip.cpp` and
  others) `#include <emmintrin.h>`/`<immintrin.h>` directly whenever
  `_MSC_VER` is defined - true for clang-cl too. `CMakeLists.txt` passes
  `/clang:-mssse3` etc. to the affected files when
  `CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MSVC`. If a new vendored SIMD
  file breaks clang-cl the same way, add it to that block rather than
  guessing at other fixes.
- **ARM64EC has the same class of problem, one level deeper**: MSVC defines
  `_M_X64` for ARM64EC builds too (it's x64-ABI-compatible), but its
  `emmintrin.h`/`immintrin.h` refuse to be included there except via
  `<intrin.h>`. `Source/LibTIFF4/tif_predict.c` and
  `Source/LibWebP/src/dsp/cpu.c` exclude `_M_ARM64EC` from their `_M_X64`
  intrinsic-header guards for this reason (surfaced by openFrameworks'
  apothecary ARM64EC build). The `windows-arm-cross` job now cross-compiles
  ARM64 and ARM64EC with MSVC on the x64 runner and does not run ctest: those
  binaries cannot execute on `windows-latest`. If another vendored file hits
  `error C1189` under an ARM64EC build, it's the same fix: exclude
  `_M_ARM64EC` from the guard, fall back to the portable path.
- **`windows-mingw` in the main matrix has `BUILD_OPENEXR=OFF` forced** for
  its C++17+ entries. `BUILD_OPENEXR` defaults ON for C++17+ (since #85), but
  on this job's specific preinstalled mingw toolchain the test binary hangs
  *before `main()` even runs* - confirmed to be some global/static
  constructor or module-load-time initializer somewhere in the
  OpenEXR/Imath/IlmThread/OpenEXRCore/LibDeflate chain, not yet root-caused.
  `windows-msys2` (a current, real MSYS2 toolchain, not whatever ships
  preinstalled on `windows-latest`) does *not* hit this, so it's toolchain-
  specific rather than a general mingw/GCC issue. **This needs someone with
  real Windows + a debugger attached to a hung process to actually root-cause** -
  remote CI round-trips (~25 min each, ctest's timeout) aren't enough to
  bisect a pre-main hang, since ctest doesn't surface a killed test's
  captured stdout at all (a file-based diagnostic log is more reliable if
  you need to retry this - write progress to a file, dump it in a
  `if: always()` step, don't rely on `--verbose`'s captured stdout).
- **`windows-msys2` matrix covers MINGW64/UCRT64/CLANG64, deliberately not
  CLANGARM64.** CLANGARM64's own toolchain binaries (cmake.exe, ninja.exe,
  etc.) are native ARM64 - they can't execute on the x86_64 `windows-latest`
  runner at all ("Exec format error"). This needs a real ARM64 Windows
  runner (e.g. `windows-11-arm`), not a cross-compile setup - MSYS2's
  environments aren't cross-compilers, they're native-per-arch toolchains.

## Verification convention for bug fixes

For any memory-safety fix, the pattern used throughout this repo's recent
history (and expected for new ones):

1. Build with ASan: `-DCMAKE_CXX_FLAGS="-fsanitize=address -g"
   -DCMAKE_C_FLAGS="-fsanitize=address -g"
   -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"`.
2. Reproduce the bug with a **crafted minimal input**, not just "load some
   real file and hope" - most of these bugs need a byte layout close to a
   buffer boundary to actually trip ASan's redzones.
3. Confirm on a **pre-fix baseline** via `git worktree add <path>
   origin/master` (or the specific commit before your fix) built the same
   way, to prove the bug was real and the PoC actually exercises it - not
   just that your fix compiles.
4. Confirm **zero ASan reports on the fixed build** with the same PoC, plus
   the full `TestAPI` suite still passing (no behavior change for
   well-formed input).
5. Many internal parsing functions (`read_iptc_profile`,
   `jpeg_read_exif_profile`, etc.) are non-`static` and declared in
   `Source/Metadata/FreeImageTag.h` - they can be called directly from a
   small standalone harness linked against `libFreeImage.a` without needing
   to construct a full valid file, which is much faster to iterate on than
   round-tripping through `FreeImage_Load()`. Functions that *are* `static`
   (e.g. `processMakerNote` in `Exif.cpp`) need either a full crafted file
   through the public API, or code-review-only verification.

## Known non-obvious traps

- **Binary-safe I/O for mass file edits.** Several source files use CRLF line
  endings and non-UTF8 bytes (ISO-8859-1/cp1252) in author-credit comments.
  Always edit/rewrite with binary-safe I/O (Python `'rb'`/`'wb'`, no text-mode
  newline translation) for any scripted multi-file edit - text-mode tools
  will silently mangle line endings or corrupt non-ASCII bytes.
- **`.gitignore` has broad `bin/`, `lib/`, `config.make` patterns** left over
  from the original SourceForge-era project layout, which will silently
  swallow legitimately-committed files that happen to live under a
  similarly-named path (this has bitten `Source/LibDeflate/lib/`'s real
  sources before, and the openFrameworks example's `bin/data/` test images).
  If `git add` doesn't pick up a file you expect it to, check
  `git check-ignore -v <path>` before assuming it's a new-file mistake.
- **NVD/CVE writeups for this codebase have been unreliable** - several
  "vulnerable" functions turned out to be already-fixed or safe-by-
  construction on inspection, and conversely some CVEs traced to a
  completely different function than their NVD description implied. Always
  verify against this repo's actual current source, not the CVE description
  or a web search summary.

## Where things stand (security)

Issue #35 tracks a list of ~25 CVEs. As of the fixes referenced there, every
FreeImage-own bug in that list is resolved, confirmed already-safe, or
avoidable via `USE_SYSTEM_*`; several were in the bundled OpenEXR 2.2-era
code or OpenJPEG's `j2k.c`, fully obsoleted by later vendor bumps. Check that
issue's comment thread for the current per-CVE status before assuming
something in the list is still open.
