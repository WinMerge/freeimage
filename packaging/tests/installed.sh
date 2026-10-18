#!/bin/sh
set -eu
task_tmp_dir=$(mktemp -d)
trap 'rm -rf "$task_tmp_dir"' EXIT
cc "$(dirname "$0")/smoke.c" $(pkg-config --cflags --libs freeimage) -o "$task_tmp_dir/smoke"
"$task_tmp_dir/smoke"
cmake -S "$(dirname "$0")" -B "$task_tmp_dir/cmake"
cmake --build "$task_tmp_dir/cmake"
"$task_tmp_dir/cmake/smoke"
