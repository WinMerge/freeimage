#!/usr/bin/env bash
# Compare vendored library versions under Source/ with upstream releases.
# With --file-issue, open or update one GitHub issue when any library is behind.
# LibJXR is skipped: it has no maintained upstream.
set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MARKER='<!-- upstream-version-check -->'
ISSUE_TITLE='Vendored libraries have newer upstream releases'
TOKEN="${GH_TOKEN:-${GITHUB_TOKEN:-}}"
BEHIND_FILE="$(mktemp)"
ERROR_FILE="$(mktemp)"
BODY_FILE="$(mktemp)"
trap 'rm -f "$BEHIND_FILE" "$ERROR_FILE" "$BODY_FILE"' EXIT

gh_get() {
    local url="$1"
    local -a args
    args=(-fsSL -H "Accept: application/vnd.github+json" -H "User-Agent: freeimage-upstream-check")
    if [ -n "$TOKEN" ]; then
        args+=(-H "Authorization: Bearer $TOKEN")
    fi
    curl "${args[@]}" "$url"
}

plain_version() {
    local value="$1"
    value="${value#v}"
    value="${value#V}"
    printf '%s\n' "$value" | sed -E 's/^([0-9]+(\.[0-9]+)*).*/\1/'
}

is_prerelease() {
    printf '%s\n' "$1" | grep -Eqi 'rc|alpha|beta|preview|pre[0-9]'
}

# Return 0 when $1 is strictly newer than $2.
version_gt() {
    local IFS='.'
    local -a left right
    local i max a b
    read -r -a left <<< "$(plain_version "$1")"
    read -r -a right <<< "$(plain_version "$2")"
    max=${#left[@]}
    if [ "${#right[@]}" -gt "$max" ]; then
        max=${#right[@]}
    fi
    for ((i = 0; i < max; i++)); do
        a="${left[$i]:-0}"
        b="${right[$i]:-0}"
        a="${a%%[^0-9]*}"
        b="${b%%[^0-9]*}"
        a="${a:-0}"
        b="${b:-0}"
        if [ "$a" -gt "$b" ]; then
            return 0
        fi
        if [ "$a" -lt "$b" ]; then
            return 1
        fi
    done
    return 1
}

read_match() {
    local file="$1"
    local pattern="$2"
    local line
    line="$(grep -E "$pattern" "$ROOT/$file" | head -n 1)" || return 1
    if [[ ! "$line" =~ $pattern ]]; then
        return 1
    fi
    plain_version "${BASH_REMATCH[1]}"
}

read_libraw() {
    local file="$ROOT/Source/LibRawLite/libraw/libraw_version.h"
    local major minor patch
    major="$(sed -nE 's/^#define[[:space:]]+LIBRAW_MAJOR_VERSION[[:space:]]+([0-9]+).*/\1/p' "$file")"
    minor="$(sed -nE 's/^#define[[:space:]]+LIBRAW_MINOR_VERSION[[:space:]]+([0-9]+).*/\1/p' "$file")"
    patch="$(sed -nE 's/^#define[[:space:]]+LIBRAW_PATCH_VERSION[[:space:]]+([0-9]+).*/\1/p' "$file")"
    if [ -z "$major" ] || [ -z "$minor" ] || [ -z "$patch" ]; then
        return 1
    fi
    printf '%s.%s.%s\n' "$major" "$minor" "$patch"
}

github_latest() {
    local repo="$1"
    local json tag url best="" candidate
    if json="$(gh_get "https://api.github.com/repos/${repo}/releases/latest" 2>/dev/null)"; then
        tag="$(printf '%s' "$json" | jq -r '.tag_name // empty')"
        url="$(printf '%s' "$json" | jq -r '.html_url // empty')"
        if [ -n "$tag" ] && ! is_prerelease "$tag"; then
            printf '%s %s\n' "$tag" "${url:-https://github.com/${repo}/releases/latest}"
            return 0
        fi
    fi
    json="$(gh_get "https://api.github.com/repos/${repo}/tags?per_page=100")" || return 1
    while read -r candidate; do
        [ -n "$candidate" ] || continue
        is_prerelease "$candidate" && continue
        plain_version "$candidate" >/dev/null || continue
        if [ -z "$best" ] || version_gt "$candidate" "$best"; then
            best="$candidate"
        fi
    done <<< "$(printf '%s' "$json" | jq -r '.[].name')"
    if [ -z "$best" ]; then
        return 1
    fi
    printf '%s https://github.com/%s/releases/tag/%s\n' "$best" "$repo" "$best"
}

gitlab_latest() {
    local project="$1"
    local encoded json tag url
    encoded="$(printf '%s' "$project" | jq -sRr @uri)"
    json="$(curl -fsSL -H "User-Agent: freeimage-upstream-check" \
        "https://gitlab.com/api/v4/projects/${encoded}/releases")" || return 1
    tag="$(printf '%s' "$json" | jq -r '.[].tag_name' | while read -r name; do
        [ -n "$name" ] || continue
        is_prerelease "$name" && continue
        printf '%s\n' "$name"
        break
    done)"
    if [ -z "$tag" ]; then
        return 1
    fi
    url="$(printf '%s' "$json" | jq -r --arg tag "$tag" '.[] | select(.tag_name == $tag) | .html_url' | head -n 1)"
    printf '%s %s\n' "$tag" "${url:-https://gitlab.com/${project}/-/releases/${tag}}"
}

ijg_latest() {
    local page best="" candidate num letter best_key="" key
    page="$(curl -fsSL -H "User-Agent: freeimage-upstream-check" "https://www.ijg.org/files/")" || return 1
    while read -r candidate; do
        [ -n "$candidate" ] || continue
        num="${candidate%%[a-z]*}"
        letter="${candidate#"$num"}"
        key="$(printf '%05d%s' "$num" "$letter")"
        if [ -z "$best" ] || [[ "$key" > "$best_key" ]]; then
            best="$candidate"
            best_key="$key"
        fi
    done <<< "$(printf '%s\n' "$page" | grep -oE 'jpegsrc\.v[0-9]+[a-z]?\.tar\.gz' | sed -E 's/jpegsrc\.v([0-9]+[a-z]?)\.tar\.gz/\1/' | sort -u)"
    if [ -z "$best" ]; then
        return 1
    fi
    printf '%s https://www.ijg.org/files/\n' "$best"
}

record() {
    local name="$1" vendored="$2" upstream="$3" url="$4"
    if version_gt "$upstream" "$vendored"; then
        printf '%s\t%s\t%s\t%s\n' "$name" "$vendored" "$upstream" "$url" >> "$BEHIND_FILE"
        printf '%s: vendored %s, upstream %s (behind)\n' "$name" "$vendored" "$upstream"
    else
        printf '%s: vendored %s, upstream %s (current)\n' "$name" "$vendored" "$upstream"
    fi
}

check_lib() {
    local name="$1" kind="$2" target="$3" file="$4" pattern="$5"
    local vendored upstream_line upstream url
    if [ "$kind" = "libraw" ]; then
        vendored="$(read_libraw)" || {
            printf '%s\tvendored version not found\n' "$name" >> "$ERROR_FILE"
            printf '%s: vendored version not found\n' "$name" >&2
            return 0
        }
    else
        vendored="$(read_match "$file" "$pattern")" || {
            printf '%s\tvendored version not found in %s\n' "$name" "$file" >> "$ERROR_FILE"
            printf '%s: vendored version not found in %s\n' "$name" "$file" >&2
            return 0
        }
    fi
    case "$kind" in
        github|libraw) upstream_line="$(github_latest "$target")" || upstream_line="" ;;
        gitlab) upstream_line="$(gitlab_latest "$target")" || upstream_line="" ;;
        ijg) upstream_line="$(ijg_latest)" || upstream_line="" ;;
        *) upstream_line="" ;;
    esac
    if [ -z "$upstream_line" ]; then
        printf '%s\tupstream lookup failed\n' "$name" >> "$ERROR_FILE"
        printf '%s: upstream lookup failed\n' "$name" >&2
        return 0
    fi
    upstream="${upstream_line%% *}"
    url="${upstream_line#* }"
    record "$name" "$vendored" "$upstream" "$url"
}

write_body() {
    {
        printf '%s\n\n' "$MARKER"
        printf '%s\n' "A scheduled check compared the versions in \`Source/\` with each project's latest stable release."
        printf '%s\n\n' "A newer tag is not always the branch this tree vendors. OpenEXR 3.3.x can be current for this fork while a later major release is the newest tag."
        printf '%s\n' "| Library | Vendored | Latest upstream |"
        printf '%s\n' "| --- | --- | --- |"
        while IFS=$'\t' read -r name vendored upstream url; do
            [ -n "${name:-}" ] || continue
            printf '| %s | %s | [%s](%s) |\n' "$name" "$vendored" "$upstream" "$url"
        done < "$BEHIND_FILE"
        if [ -s "$ERROR_FILE" ]; then
            printf '\n%s\n\n' "Checks that did not complete:"
            while IFS=$'\t' read -r name message; do
                [ -n "${name:-}" ] || continue
                printf -- '- %s: %s\n' "$name" "$message"
            done < "$ERROR_FILE"
        fi
        printf '\n%s\n' "Workflow: \`.github/workflows/upstream-versions.yml\`."
    } > "$BODY_FILE"
}

file_issue() {
    local repo="${GITHUB_REPOSITORY:-}"
    if [ ! -s "$BEHIND_FILE" ]; then
        printf '%s\n' "No vendored library is behind a stable upstream release."
        return 0
    fi
    write_body
    if [ -z "$repo" ] || [ -z "$TOKEN" ]; then
        cat "$BODY_FILE"
        printf '\n%s\n' "GH_TOKEN and GITHUB_REPOSITORY are not set, so no issue was filed."
        return 0
    fi
    local search number existing_url existing_body
    search="$(curl -fsSL -G \
        -H "Accept: application/vnd.github+json" \
        -H "Authorization: Bearer $TOKEN" \
        -H "User-Agent: freeimage-upstream-check" \
        --data-urlencode "q=repo:${repo} is:issue is:open in:title \"${ISSUE_TITLE}\"" \
        "https://api.github.com/search/issues")" || return 1
    number="$(printf '%s' "$search" | jq -r '.items[0].number // empty')"
    if [ -n "$number" ]; then
        existing_body="$(printf '%s' "$search" | jq -r '.items[0].body // empty')"
        existing_url="$(printf '%s' "$search" | jq -r '.items[0].html_url')"
        if [ "$existing_body" = "$(cat "$BODY_FILE")" ]; then
            printf 'Open issue already matches: %s\n' "$existing_url"
            return 0
        fi
        jq -n --rawfile body "$BODY_FILE" --arg title "$ISSUE_TITLE" '{title: $title, body: $body}' \
            | curl -fsSL -X PATCH \
                -H "Accept: application/vnd.github+json" \
                -H "Authorization: Bearer $TOKEN" \
                -H "User-Agent: freeimage-upstream-check" \
                -H "Content-Type: application/json" \
                --data-binary @- \
                "https://api.github.com/repos/${repo}/issues/${number}" >/dev/null
        printf 'Updated %s\n' "$existing_url"
        return 0
    fi
    existing_url="$(jq -n --rawfile body "$BODY_FILE" --arg title "$ISSUE_TITLE" '{title: $title, body: $body}' \
        | curl -fsSL -X POST \
            -H "Accept: application/vnd.github+json" \
            -H "Authorization: Bearer $TOKEN" \
            -H "User-Agent: freeimage-upstream-check" \
            -H "Content-Type: application/json" \
            --data-binary @- \
            "https://api.github.com/repos/${repo}/issues" \
        | jq -r '.html_url')"
    printf 'Opened %s\n' "$existing_url"
}

FILE_ISSUE=0
if [ "${1:-}" = "--file-issue" ]; then
    FILE_ISSUE=1
fi

check_lib "OpenEXR" github "AcademySoftwareFoundation/openexr" "Source/OpenEXR/OpenEXRConfig.h" 'OPENEXR_VERSION_STRING[[:space:]]+"([0-9.]+)"'
check_lib "Imath" github "AcademySoftwareFoundation/Imath" "Source/Imath/ImathConfig.h" 'IMATH_VERSION_STRING[[:space:]]+"([0-9.]+)"'
check_lib "libdeflate" github "ebiggers/libdeflate" "Source/LibDeflate/libdeflate.h" 'LIBDEFLATE_VERSION_STRING[[:space:]]+"([0-9.]+)"'
check_lib "OpenJPEG" github "uclouvain/openjpeg" "Source/LibOpenJPEG/opj_config_private.h" 'OPJ_PACKAGE_VERSION[[:space:]]+"([0-9.]+)"'
check_lib "libpng" github "pnggroup/libpng" "Source/LibPNG/png.h" 'PNG_LIBPNG_VER_STRING[[:space:]]+"([0-9.]+)"'
check_lib "LibRaw" libraw "LibRaw/LibRaw" "Source/LibRawLite/libraw/libraw_version.h" 'LIBRAW_MAJOR_VERSION[[:space:]]+([0-9]+)'
check_lib "libtiff" gitlab "libtiff/libtiff" "Source/LibTIFF4/tiffvers.h" 'TIFFLIB_VERSION_STR_MAJ_MIN_MIC[[:space:]]+"([0-9.]+)"'
check_lib "libwebp" github "webmproject/libwebp" "Source/LibWebP/NEWS" 'version[[:space:]]+([0-9.]+)'
check_lib "zlib" github "madler/zlib" "Source/ZLib/zlib.h" 'ZLIB_VERSION[[:space:]]+"([0-9.]+)"'
check_lib "libjpeg (IJG)" ijg "ijg" "Source/LibJPEG/jversion.h" 'JVERSION[[:space:]]+"([0-9]+)'

if [ "$FILE_ISSUE" -eq 1 ]; then
    file_issue
fi
