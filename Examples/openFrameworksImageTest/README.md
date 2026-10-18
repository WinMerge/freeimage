openFrameworks Image Test
==========================

A minimal [openFrameworks](https://openframeworks.cc) app that loads every
image FreeImage's own `TestAPI` ships as a checked-in fixture (`sample.png`,
`exif.jpg`, `exif.jxr` - copied into `bin/data/` here) through `ofImage`,
prints PASS/FAIL and the decoded dimensions for each to the console, and
draws thumbnails with the same status on screen. Press `r` to reload.

This exercises the same integration path as this repo's
`openFrameworks image format tests` CI job
(`.github/workflows/cmake.yml`): openFrameworks' own `ofImage` is backed by
a bundled copy of FreeImage, so this app is really testing *this build of
FreeImage* through oF's own image loader - not the FreeImage API directly
(see `Examples/Generic` for direct-API examples instead).

## Building

openFrameworks projects normally live inside an oF checkout's
`apps/myApps/` folder, with paths back to `OF_ROOT` baked into their
`Makefile`. This folder isn't inside a checkout, so either:

- Copy or symlink this folder into `openFrameworks/apps/myApps/openFrameworksImageTest`, or
- Build with `make OF_ROOT=/path/to/openFrameworks`

By default oF ships with its own bundled copy of FreeImage
(`openFrameworks/libs/FreeImage`). To build against *this* repo's FreeImage
instead - the whole point of this example - swap it in first, matching what
the CI job above does:

```bash
# from a FreeImage checkout, build a static library
cmake -S . -B build -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build

# then, for a macOS oF checkout at $OF_ROOT:
cp Source/FreeImage.h "$OF_ROOT/libs/FreeImage/include/FreeImage.h"
cp build/libFreeImage.a "$OF_ROOT/libs/FreeImage/lib/macos/FreeImage.xcframework/macos-arm64_x86_64/FreeImage.a"
```

(the exact library path under `libs/FreeImage/lib/` depends on platform -
see oF's own `libs/FreeImage` folder layout for Linux/Windows equivalents)

Then, from this folder:

```bash
make
make RunRelease   # or: make run
```

## What a failure looks like

If a format this build of FreeImage should support fails to decode, that
image's thumbnail draws as a red box and the console logs
`[ error ] FreeImageTest: N of 3 images failed to load.` - useful as a quick
sanity check after changing plugin code, independent of `TestAPI`'s
non-visual test suite.
