VENDOR_DIR="Source/ZLib"
SOURCE_SUBDIRS="."
ROOT_FILES=""
CMAKE_VAR="FreeImage_ZLIB_SRCS"
URL_TEMPLATE="https://github.com/madler/zlib/archive/refs/tags/v{version}.tar.gz"
# Flat layout - merge-copy so ZLib.2017.vcxproj* (FreeImage's own, not
# upstream) survive. zlib >= 1.3 needs -DHAVE_UNISTD_H on non-Windows
# (see CMakeLists.txt comment) - re-check this is still needed/correct
# after bumping.
