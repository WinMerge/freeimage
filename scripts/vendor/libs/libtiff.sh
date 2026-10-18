VENDOR_DIR="Source/LibTIFF4"
SOURCE_SUBDIRS="."
UPSTREAM_SUBDIR="libtiff"
ROOT_FILES=""
CMAKE_VAR="FreeImage_LIBTIFF_SRCS"
URL_TEMPLATE="https://gitlab.com/libtiff/libtiff/-/archive/v{version}/libtiff-v{version}.tar.gz"
# Modern libtiff moved its actual library source into a libtiff/ subdir
# (CLI tools like tiffcp/tiffdump moved to tools/, tests to test/) -
# UPSTREAM_SUBDIR points the "." flat-merge logic at that subdir instead
# of the tarball root, while still landing files flat under Source/
# LibTIFF4 with no extra nesting, matching this repo's existing layout.
# LibTIFF4.2017.vcxproj* (FreeImage's own) survive since merge is
# update-only. tif_config.h/tiffconf.h/tiffvers.h are checked-in
# generated headers (upstream ships .in/.cmake.in templates instead,
# which are correctly skipped since they're not in the known file list).
