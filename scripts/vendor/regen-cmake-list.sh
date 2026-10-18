#!/bin/bash
# Regenerates a `set(<VAR> ... )` source file list block in a CMakeLists.txt
# from whatever .c/.h/.cpp/.hpp files currently exist under the given
# vendor subdirs. Used by bump-lib.sh; not normally run directly.
#
# Usage: regen-cmake-list.sh <CMakeLists.txt path> <CMAKE_VAR> <vendor dir> "<subdir1> <subdir2> ..."

set -euo pipefail

CMAKE_FILE="$1"
VAR_NAME="$2"
VENDOR_PATH="$3"
SOURCE_SUBDIRS="$4"

REPO_ROOT="$(cd "$(dirname "$CMAKE_FILE")" && pwd)"

TMP_LIST=$(mktemp)
trap 'rm -f "$TMP_LIST"' EXIT

for sub in $SOURCE_SUBDIRS; do
	if [ "$sub" = "." ]; then
		# "." means this library's original layout is flat (no subdirs) -
		# stay at maxdepth 1 so any new top-level dirs a newer release adds
		# (contrib/, ci/, arch-specific SIMD dirs like arm/intel/mips/...)
		# don't get swept into the build. Named subdirs below are trusted
		# to recurse fully, since those are the library's real source tree.
		BASE="$VENDOR_PATH"
		DEPTH_ARGS="-maxdepth 1"
	else
		BASE="$VENDOR_PATH/$sub"
		DEPTH_ARGS=""
	fi
	find "$BASE" $DEPTH_ARGS \( -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp" \) -print
done | sed "s|^$REPO_ROOT/|./|" | sort > "$TMP_LIST"

FORMATTED=$(awk '
	BEGIN { line = "        "; width = 96 }
	{
		cand = line $0 " "
		if (length(cand) > width) {
			print line
			line = "        " $0 " "
		} else {
			line = cand
		}
	}
	END { if (line != "        ") print line }
' "$TMP_LIST")

TMP_BLOCK=$(mktemp)
trap 'rm -f "$TMP_LIST" "$TMP_BLOCK"' EXIT
{
	echo "    set($VAR_NAME"
	printf '%s\n' "$FORMATTED"
	echo "    )"
} > "$TMP_BLOCK"

# BSD awk (macOS default) rejects embedded newlines in -v strings, so the
# replacement block is read from a file instead of passed as a variable.
awk -v marker="set($VAR_NAME" -v blockfile="$TMP_BLOCK" '
	BEGIN { skip = 0 }
	{
		if (!skip && index($0, marker) > 0) {
			while ((getline line < blockfile) > 0) {
				print line
			}
			close(blockfile)
			skip = 1
			next
		}
		if (skip) {
			line = $0
			gsub(/^[ \t]+|[ \t]+$/, "", line)
			if (line == ")") {
				skip = 0
			}
			next
		}
		print
	}
' "$CMAKE_FILE" > "$CMAKE_FILE.tmp"
mv "$CMAKE_FILE.tmp" "$CMAKE_FILE"

echo "  wrote $(wc -l < "$TMP_LIST" | tr -d ' ') entries"
