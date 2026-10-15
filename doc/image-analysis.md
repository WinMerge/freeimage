# Image analysis and simple tone mapping

These additive APIs cover gray8 (MINISBLACK), RGB24/RGBA32, UINT16/RGB16/RGBA16,
and FLOAT/DOUBLE/RGBF/RGBAF. Other formats return FALSE/NULL without conversion.
They adapt FreeImageRe's feature set to FreeImage 3's types without its new
YUV, complex-float or C++20 infrastructure.

- `FreeImage_FindMinMax`: scalar range or Rec.709 RGB luminance range, ignoring
  non-finite brightness. Optional pixel pointers borrow source memory and remain
  valid only while that bitmap storage exists. Failure leaves outputs unchanged.
- `FreeImage_FindMinMaxValue`: independent finite channel minima/maxima. Output
  storage must match the input pixel type (a BYTE for gray8; native three/four
  byte pixel layout for RGB24/RGBA32; WORD/FIRGB16/FIRGBA16/float/double/FIRGBF/
  FIRGBAF otherwise). Alpha participates here, but not in luminance extrema.
- `FreeImage_TmoClamp`: divide colour by the supplied positive maximum, otherwise
  the type's nominal range (255, 65535, or 1), then clamp to 8-bit. NaN and
  negative infinity map to 0; positive infinity maps to 255.
- `FreeImage_TmoLinear`: stretch the finite luminance range into 8-bit output.
  An optional positive maximum overrides the white point. Constant finite images
  map to black. All-non-finite inputs fail. RGB may clip; no gamma correction is
  implied. Alpha uses its nominal range and is never contrast-stretched.
- `FreeImage_GetHistogramEx`: caller chooses the bin count, finite numeric range,
  and RGB luminance or R/G/B/A channel. Out-of-range/non-finite samples are
  ignored. The upper bound is included in the last bin. Invalid arguments and
  counter overflow fail without modifying the caller's histogram.

Tone mapping returns a new bitmap (gray8/RGB24/RGBA32), preserves metadata, and
never changes the source. FITMO_CLAMP/FITMO_LINEAR also work through
FreeImage_ToneMapping; first_param is the maximum, second_param is unused.
Existing tone-mapping enum values and FreeImage_GetHistogram are unchanged.

The histogram API is deliberately named GetHistogramEx rather than pretending
binary compatibility with FreeImageRe's differently shaped MakeHistogram API.
