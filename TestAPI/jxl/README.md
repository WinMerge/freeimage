# JPEG XL decode fixtures

`JpegXLTest` decodes these and compares them with `reference.png` (a 60x54
RGBA downscale of `TestAPI/sample.png`). They cover decoder paths that the
lossless roundtrips in `testJpegXL.cpp` do not: VarDCT, progressive DC and
the SIMD upsampling stages (issue #211).

Generated with `cjxl` 0.12.0:

```sh
sips -z 54 60 ../sample.png --out reference.png
cjxl reference.png lossy.jxl -d 1 -e 3
cjxl reference.png upsampled2x.jxl -d 1 -e 3 --resampling=2 --ec_resampling=2
cjxl reference.png upsampled4x.jxl -d 2 -e 3 --resampling=4 --ec_resampling=4
cjxl reference.png alpha-upsampled.jxl -d 1 -e 3 --resampling=1 --ec_resampling=2
cjxl reference.png progressive.jxl -d 1 -e 3 -p --progressive_dc=1
```

If you regenerate them, recheck the error thresholds in `testJpegXL.cpp`.
