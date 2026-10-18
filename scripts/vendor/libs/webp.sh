VENDOR_DIR="Source/LibWebP"
SOURCE_SUBDIRS="src sharpyuv"
ROOT_FILES="AUTHORS COPYING ChangeLog NEWS PATENTS README.md"
CMAKE_VAR="FreeImage_WEBP_SRCS"
URL_TEMPLATE="https://github.com/webmproject/libwebp/archive/refs/tags/v{version}.tar.gz"
# sharpyuv/ was split out of src/enc in 1.3.x and is now a hard dependency
# of the encoder - already handled here, but flag any *new* top-level
# dirs a future release adds.
