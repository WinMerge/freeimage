# Complete the OpenJPEG 2.5.4 Windows project source list

Companion to the existing #182 source upgrade. The older attachment removes raw.c but leaves it referenced in LibOpenJPEG.2017.vcxproj, and the new source units are not all listed. This small patch reconciles the project compilation/header items with the same 2.5.4 source set already used by the fork and GNU make.

Apply after the existing #182 source patch with `gzip -dc openjpeg-2.5.4-project.patch.gz | patch -E -p0`. It changes project metadata only, not the library version or source implementation. Project XML and every referenced compilation source were checked; an actual Windows build remains necessary.
