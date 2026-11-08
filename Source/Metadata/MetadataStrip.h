#ifndef FREEIMAGE_METADATA_STRIP_H
#define FREEIMAGE_METADATA_STRIP_H

#include "../FreeImage.h"

// Build with FREEIMAGE_METADATA_STRIP=0 to compile metadata stripping out.
// FREEIMAGE_STRIP is then accepted and ignored, and FreeImage_StripMetadata
// stays exported but returns FALSE, so the ABI does not change.
#ifndef FREEIMAGE_METADATA_STRIP
#define FREEIMAGE_METADATA_STRIP 1
#endif

namespace FreeImageInternal {

#if FREEIMAGE_METADATA_STRIP

// Per-operation policy for optional metadata readers, including nested loads.
// Decoder libraries may still parse container fields needed to decode pixels.
bool IsStrippingMetadata();
inline bool WantsMetadataStrip(int flags) {
    return (flags & FREEIMAGE_STRIP) != 0 || IsStrippingMetadata();
}
class MetadataStripScope {
    bool previous_;
    MetadataStripScope(const MetadataStripScope &);
    MetadataStripScope &operator=(const MetadataStripScope &);
public:
    explicit MetadataStripScope(bool strip);
    ~MetadataStripScope();
};

// Owns a separate header/palette/animation model, borrowing the source pixels.
// FreeImage_Unload releases the view without freeing the borrowed pixels.
FIBITMAP *CreateStrippedView(FIBITMAP *source);

#else // !FREEIMAGE_METADATA_STRIP

// Constant false lets every reader-side check fold away.
inline bool IsStrippingMetadata() { return false; }
inline bool WantsMetadataStrip(int) { return false; }
class MetadataStripScope {
public:
    explicit MetadataStripScope(bool) {}
};
inline FIBITMAP *CreateStrippedView(FIBITMAP *) { return NULL; }

#endif // FREEIMAGE_METADATA_STRIP

}
#endif
