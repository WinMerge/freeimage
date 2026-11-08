# Large-file test data releases

The `Large-file test fixture assets` workflow publishes synthetic images under
`test-fixtures-io64-v1`, a **prerelease for test data**, separate from FreeImage
software versions. Merging the generator/workflow PR into `master` validates
packaging on Linux, macOS and Windows, then publishes once. Pull requests only
validate packaging; they cannot create tags or releases.

Each of the 91 fixtures is a **separate `.tar.gz` release asset**, with its own
SHA-256 and compressed size in `manifest.json`. Shared assets are the manifest,
`SHA256SUMS` and `README.md`. No binary images are checked into the repository.
The publisher generates each archive directly from sparse extents, without
creating large images on disk. The archives contain real TIFF files with data
beyond 2 GiB and 4 GiB, including classic TIFF, BigTIFF, both byte orders, and
high-offset Exif and thumbnails. Malformed examples include truncated headers,
directories and strips, invalid offsets, overflowing counts and zero dimensions.
Small PNG/BMP files exercise fallback loaders. Nested multilingual paths contain
valid and malformed inputs across ten filename variants, including combining
characters and emoji.

The corpus has roughly 30 GiB of total logical file data, but tests never need
all of it present. The runner downloads one asset, extracts it into a temporary
directory, runs the decoder, then removes both the download and extracted image
before starting the next case. Cleanup also runs after failures/timeouts. The
extractor preserves holes and explicitly marks files sparse on Windows. There
is no persistent payload cache. Local runs can use `--payload-dir` to reuse
previously downloaded archives; only the current image is extracted.
Transient HTTP errors (408, 429, 500, 502, 503, 504) and interrupted connections
are retried up to four total attempts, with 1, 2 and 4 second delays. Exhausted
downloads, checksum errors and decoder errors still fail the case; downloaded
payloads remain limited to 1 MiB and are verified before extraction.

After publication, download the small manifest and run cases sequentially:

```sh
gh release download test-fixtures-io64-v1 --repo danoli3/FreeImage \
  --pattern manifest.json --pattern SHA256SUMS
python scripts/tests/io64-fixtures.py run --manifest manifest.json \
  --release-url https://github.com/danoli3/FreeImage/releases/download/test-fixtures-io64-v1 \
  -- DECODER '{format}' '{expect}' '{path}'
```

`DECODER` is the test executable supplied by the separate loader regression PR.
It must return zero when the fixture behaves as expected; `expect` is `decode`
or `reject`. The manifest provides 1x1 dimensions and RGB (18, 52, 86), with
additional metadata/thumbnail expectations for the high-offset metadata case.
Select individual inputs with repeatable `--case valid/bigtiff-strip-above-4g.tif`
options before `--`. A decoder that fails is reported, cleaned up, and testing
continues; the runner returns nonzero if any case fails. The default per-case
timeout is 30 seconds, configurable with `--timeout`.

PR #197's `Published large-file image tests` workflow runs this corpus on Linux
with ASan, macOS C++11, MSVC x64 DLL and MSVC Win32 builds. Its decoder checks
filename and stream loading, native pixels, high-offset Exif/thumbnails and
TIFF header-only loading. Windows opens multilingual filenames as UTF-16 and
passes the file through the handle API; #197 does not add a Unicode filename
entry point. PNG/BMP use the existing loaders because #197's wide API is TIFF-only.

The optional `{known}` placeholder passes the recorded behavior to a decoder.
With `--allow-known-failures`, exit code 3 is reported as `XFAIL` only when the
manifest lists a known behavior for that input. Other errors still fail, and
successful rejection of a previously accepted malformed file passes normally.
The summary separates passing cases, known failures and unexpected failures;
the flag is off by default. This preserves visibility of the four baseline
acceptance bugs while allowing unrelated regressions to block CI.

For Actions, add `--manifest-sha256 REVIEWED_HASH` to pin the reviewed manifest.
Each payload is checked against its individual hash before extraction. GNU tar
and macOS bsdtar can also unpack one asset. The `extract` subcommand accepts
one downloaded asset, `--manifest` and a fresh `--output-dir`. Avoid copying
images with tools that materialize sparse holes.

Four malformed strip/offset cases return a bitmap on the unmodified PR #197
head; `known_behavior` records the tested commit and API. Their intended result
stays `reject`. Publishing this data does not include loader fixes or claim
every existing loader already rejects every malformed input.

Re-running publication checks identical checksums and leaves that release
untouched. Different contents require a new fixture tag, such as
`test-fixtures-io64-v2`, selected through **Run workflow** on `master`.
Software version tags such as `3.19.29` are rejected. Incomplete drafts or tags
without releases require inspection instead of automatic replacement. Fixture
tags do not match numeric software-release triggers and are never marked as
the latest software release.

Local packaging checks and generation require only Python 3.10 or newer:

```sh
python scripts/tests/test-io64-fixtures.py
python scripts/tests/io64-fixtures.py build --output-dir /tmp/io64-assets
```

All inputs are created from scratch and released under CC0-1.0. There are no
customer images, downloaded third-party samples or committed large binaries.
