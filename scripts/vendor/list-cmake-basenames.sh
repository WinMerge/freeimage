#!/bin/bash
# Prints the basenames (one per line) of every file path inside a
# `set(<VAR> ... )` block in a CMakeLists.txt. Used by bump-lib.sh for
# flat-layout libraries to know which files it's allowed to update.
#
# Usage: list-cmake-basenames.sh <CMakeLists.txt path> <CMAKE_VAR>

set -euo pipefail

CMAKE_FILE="$1"
VAR_NAME="$2"

awk -v marker="set($VAR_NAME" '
	BEGIN { skip = 0 }
	{
		if (!skip && index($0, marker) > 0) { skip = 1; next }
		if (skip) {
			line = $0
			gsub(/^[ \t]+|[ \t]+$/, "", line)
			if (line == ")") { skip = 0; next }
			n = split($0, parts, /[ \t]+/)
			for (i = 1; i <= n; i++) {
				if (parts[i] != "") print parts[i]
			}
		}
	}
' "$CMAKE_FILE" | sed 's|.*/||' | sort -u
