VENDOR_DIR="Source/OpenEXR"
SOURCE_SUBDIRS=""
ROOT_FILES=""
CMAKE_VAR=""
URL_TEMPLATE=""
# OpenEXR uses FreeImage's historical IlmImf directory name and flat Imath
# headers, plus separately embedded libdeflate, Zstandard, and OpenJPH codecs.
# Do not run the generic overlay helper: follow scripts/vendor/NOTES.md and
# reconcile both CMakeLists.txt and Makefile.srcs with upstream's library lists.
