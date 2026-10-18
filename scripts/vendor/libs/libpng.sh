VENDOR_DIR="Source/LibPNG"
SOURCE_SUBDIRS="."
ROOT_FILES=""
CMAKE_VAR="FreeImage_LIBPNG_SRCS"
URL_TEMPLATE="https://github.com/pnggroup/libpng/archive/refs/tags/v{version}.tar.gz"
# Flat layout - merge-copy so LibPNG.2017.vcxproj* survive. pnglibconf.h
# is a generated/checked-in config header, not part of the upstream
# tarball - diff it against pnglibconf.h.prebuilt after bumping in case
# new PNG_*_SUPPORTED toggles were added.
