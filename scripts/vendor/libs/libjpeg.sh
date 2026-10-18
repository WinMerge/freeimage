VENDOR_DIR="Source/LibJPEG"
SOURCE_SUBDIRS="."
ROOT_FILES=""
CMAKE_VAR="FreeImage_LIBJPEG_SRCS"
URL_TEMPLATE="https://www.ijg.org/files/jpegsrc.v{version}.tar.gz"
# This is IJG's original libjpeg (jversion.h says e.g. '9d'), NOT
# libjpeg-turbo - pass --version as IJG's own scheme (e.g. '9f', '10'),
# not a semver. Flat layout - merge-copy so LibJPEG.2017.vcxproj* and
# jconfig.h (checked-in, not upstream) survive.
