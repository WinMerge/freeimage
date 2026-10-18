VENDOR_DIR="Source/LibOpenJPEG"
SOURCE_SUBDIRS="."
UPSTREAM_SUBDIR="src/lib/openjp2"
ROOT_FILES=""
CMAKE_VAR="FreeImage_LIBOPENJPEG_SRCS"
URL_TEMPLATE="https://github.com/uclouvain/openjpeg/archive/refs/tags/v{version}.tar.gz"
# Modern openjpeg moved its actual library source into src/lib/openjp2/
# (a separate src/lib/openjpip/ holds the optional JPIP streaming module,
# not used here) - UPSTREAM_SUBDIR points the "." flat-merge logic at
# that subdir instead of the tarball root, landing files flat locally.
# openjp2/ itself still mixes real library sources with a few standalone
# tool/benchmark mains (bench_dwt.c, t1_generate_luts.c,
# t1_ht_generate_luts.c, test_sparse_array.c) - the known-basenames-only
# update logic correctly skips those.
#
# IMPORTANT: unlike most other bundled libs, this one's CMakeLists.txt
# source list historically only listed .c files, never .h files - so the
# known-basenames safety net won't update ANY header on its own, which
# will leave them stale relative to the freshly-bumped .c files (real
# problem hit bumping 2.0.0 -> 2.5.4: struct/prototype mismatches). After
# running bump-lib.sh, manually cross-check the full file list against
# upstream's own src/lib/openjp2/CMakeLists.txt (the `OPENJPEG_SRCS`
# variable, excluding the `if(BUILD_JPIP)` block) and copy over anything
# missing - both new files upstream added (bumping 2.0.0 -> 2.5.4 needed
# ht_dec.c, opj_malloc.c/h, sparse_array.c/h, thread.c/h,
# mqc_inl.h, opj_common.h, tls_keys.h, t1_luts.h, t1_ht_luts.h) and
# existing headers that just need their content refreshed. Also check for
# files upstream removed entirely (raw.c/raw.h in that same bump) and
# delete + unlist them - don't leave stale files mixed with a newer
# library version.
#
# opj_config.h / opj_config_private.h are checked-in generated headers
# (upstream ships .cmake.in templates instead) - only OPJ_PACKAGE_VERSION
# in opj_config_private.h needs manual updating; the rest is intentionally
# minimal/hasn't needed new entries so far, but the templates have grown
# HAVE_ALIGNED_ALLOC and friends - opj_malloc.c has safe portable
# fallbacks when none of those are defined, so this isn't required.
