#!/bin/bash
# Bumps one of FreeImage's bundled third-party libraries to a new upstream
# release: downloads the release tarball, merges it into the vendored
# source directory, and regenerates the matching CMakeLists.txt source
# file list.
#
# This only handles the mechanical part of a vendor bump. You still need
# to: build, run the test suite, check for upstream API/behavior changes
# that affect Source/FreeImage/Plugin*.cpp, and check `git status` for
# new/removed top-level source directories that this library's config
# doesn't yet list (update scripts/vendor/libs/<name>.sh if so).
#
# Usage:
#   scripts/vendor/bump-lib.sh webp --version 1.6.0
#   scripts/vendor/bump-lib.sh libraw --version 0.22.1
#   scripts/vendor/bump-lib.sh <lib> --version <ver> --url <override-url>
#
# Per-library config (source subdirs to merge, root metadata files to
# copy, the CMakeLists.txt variable name, and the download URL template)
# lives in scripts/vendor/libs/<lib>.sh - see that directory for the full
# list of supported libs and notes on any that need manual handling
# instead (OpenEXR, jxrlib).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

usage() {
	echo "Usage: $0 <lib> --version <version> [--url <override-url>]" >&2
	echo "Known libs:" >&2
	for f in "$SCRIPT_DIR"/libs/*.sh; do
		echo "  $(basename "$f" .sh)" >&2
	done
	exit 1
}

[ $# -ge 1 ] || usage
LIB="$1"; shift

VERSION=""
URL_OVERRIDE=""
while [ $# -gt 0 ]; do
	case "$1" in
		--version) VERSION="$2"; shift 2 ;;
		--url) URL_OVERRIDE="$2"; shift 2 ;;
		*) echo "Unknown argument: $1" >&2; usage ;;
	esac
done

CONFIG="$SCRIPT_DIR/libs/$LIB.sh"
[ -f "$CONFIG" ] || { echo "Unknown lib '$LIB' (no $CONFIG)" >&2; usage; }
# shellcheck source=/dev/null
source "$CONFIG"

if [ -n "$URL_OVERRIDE" ]; then
	URL="$URL_OVERRIDE"
elif [ -n "${URL_TEMPLATE:-}" ]; then
	[ -n "$VERSION" ] || { echo "--version is required" >&2; exit 1; }
	URL="${URL_TEMPLATE//\{version\}/$VERSION}"
else
	echo "No URL_TEMPLATE in $CONFIG and no --url given - this library" >&2
	echo "may need manual handling instead. See scripts/vendor/libs/$LIB.sh for notes." >&2
	exit 1
fi

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "Downloading $URL"
curl -sL -o "$TMPDIR/download.tar.gz" "$URL"
mkdir "$TMPDIR/extracted"
tar xzf "$TMPDIR/download.tar.gz" -C "$TMPDIR/extracted"

EXTRACTED_ROOT=$(find "$TMPDIR/extracted" -mindepth 1 -maxdepth 1 -type d)
COUNT=$(echo "$EXTRACTED_ROOT" | wc -l | tr -d ' ')
[ "$COUNT" = "1" ] || { echo "Expected exactly one top-level dir in the tarball, found $COUNT" >&2; exit 1; }

VENDOR_PATH="$REPO_ROOT/$VENDOR_DIR"
CMAKE_FILE="$REPO_ROOT/CMakeLists.txt"

echo "Merging source subdirs into $VENDOR_DIR"
for sub in $SOURCE_SUBDIRS; do
	if [ "$sub" = "." ]; then
		# "." means this library's original layout is flat (no subdirs),
		# and - as libpng's release tree showed - a flat upstream root
		# usually mixes real library sources with CLI tools, tests, and
		# demo programs (each with their own main()) that must NOT be
		# compiled in, plus newer releases tend to add contrib/, ci/,
		# arch-specific SIMD dirs, etc. There's no reliable way to tell
		# "real source" from "tool/test/example" by filename alone, so
		# instead of discovering files from disk, only UPDATE files whose
		# basename is already listed in this library's existing
		# CMakeLists.txt entry (CMAKE_VAR) - never add or remove files.
		# If upstream genuinely added/removed a required source file,
		# that's rare enough to warrant a human updating CMakeLists.txt
		# by hand rather than trusting a glob to guess correctly.
		# UPSTREAM_SUBDIR (optional) lets a library's config say "read the
		# flat file set from this subdir of the tarball, not the tarball
		# root itself" - e.g. modern libtiff moved its actual library
		# source into a libtiff/ subdir (CLI tools moved to tools/), while
		# this repo still keeps it flattened directly under Source/LibTIFF4
		# with no nesting, matching the pre-existing layout.
		if [ -n "${UPSTREAM_SUBDIR:-}" ]; then
			SRC="$EXTRACTED_ROOT/$UPSTREAM_SUBDIR"
		else
			SRC="$EXTRACTED_ROOT"
		fi
		DST="$VENDOR_PATH"
		mkdir -p "$DST"
		if [ -z "${CMAKE_VAR:-}" ]; then
			echo "'.' (flat) layout requires CMAKE_VAR to be set so the known file list can be read - see scripts/vendor/libs/$LIB.sh" >&2
			exit 1
		fi
		KNOWN_FILES=$("$SCRIPT_DIR/list-cmake-basenames.sh" "$CMAKE_FILE" "$CMAKE_VAR")
		for name in $KNOWN_FILES; do
			if [ -f "$SRC/$name" ]; then
				cp "$SRC/$name" "$DST/$name"
			else
				echo "  warning: '$name' is in $CMAKE_VAR but upstream no longer ships it at the root - check manually" >&2
			fi
		done
		NEW_FILES=$(find "$SRC" -maxdepth 1 -type f \( -name "*.c" -o -name "*.h" \) -exec basename {} \; | sort)
		UNKNOWN=$(comm -23 <(echo "$NEW_FILES") <(echo "$KNOWN_FILES" | tr ' ' '\n' | sort))
		if [ -n "$UNKNOWN" ]; then
			echo "  note: upstream root also has these .c/.h files, not currently compiled - review and add to CMakeLists.txt by hand if needed:"
			echo "$UNKNOWN" | sed 's/^/    /'
		fi
	else
		# Named subdir - trusted to be the library's real source tree
		# (e.g. webp's src/, libraw's internal/libraw/src), safe to merge
		# and recompute the compiled file list in full.
		SRC="$EXTRACTED_ROOT/$sub"
		DST="$VENDOR_PATH/$sub"
		if [ ! -d "$SRC" ]; then
			echo "Expected subdir '$sub' not found in upstream release - library layout may have changed, check manually." >&2
			exit 1
		fi
		mkdir -p "$DST"
		cp -R "$SRC/." "$DST/"
	fi
	echo "  merged into ${DST#"$REPO_ROOT"/}"
done

if [ -n "${ROOT_FILES:-}" ]; then
	echo "Updating root metadata files"
	for f in $ROOT_FILES; do
		if [ -f "$EXTRACTED_ROOT/$f" ]; then
			cp "$EXTRACTED_ROOT/$f" "$VENDOR_PATH/$f"
			echo "  updated $VENDOR_DIR/$f"
		else
			echo "  note: upstream no longer ships '$f' at the root (may have been renamed - check manually)"
		fi
	done
fi

# Only regenerate the CMakeLists.txt file list for named-subdir libraries
# (full source tree, safe to recompute). Flat "." libraries keep whatever
# was already there - see the note above.
if [ -n "${CMAKE_VAR:-}" ] && [ "$SOURCE_SUBDIRS" != "." ]; then
	echo "Regenerating $CMAKE_VAR in CMakeLists.txt"
	"$SCRIPT_DIR/regen-cmake-list.sh" "$CMAKE_FILE" "$CMAKE_VAR" "$VENDOR_PATH" "$SOURCE_SUBDIRS"
fi

echo
echo "Done. Remaining manual steps:"
echo "  1. Check 'git status' for new/removed top-level dirs this script doesn't know"
echo "     about yet (update scripts/vendor/libs/$LIB.sh's SOURCE_SUBDIRS if so)."
echo "  2. Configure + build: cmake -S . -B build -DBUILD_TESTS=OFF && cmake --build build"
echo "  3. Functionally test load/save round-trips for this format."
echo "  4. Check the release notes for API/behavior changes affecting"
echo "     Source/FreeImage/Plugin*.cpp."
