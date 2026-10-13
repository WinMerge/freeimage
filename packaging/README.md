# Installable Linux packages

These recipes package the danoli3 fork, using distribution codec libraries.
JPEG-XR is disabled because jxrlib is not a common dependency in the target
repositories. HEIF/AVIF is not included.

The packages install libfreeimage.so.3, FreeImage.h, freeimage.pc, and the
FreeImage CMake package. Both pkg-config and find_package consumers are
tested after installing the generated package. Linux package layout is
opt-in; existing builds retain their library naming.

## Arch Linux

Copy packaging/arch/PKGBUILD and linux-install.patch to a build directory,
then run makepkg -si as a normal user. The release tarball and packaging
patch have pinned SHA256 checksums. The patch adds Linux package layout to
release 3.19.21; it does not change codec sources.

The package name is freeimage-maintained. It conflicts with freeimage and
freeimage-git and does not claim to provide those packages. The PKGBUILD is
ready for local builds; it has not been uploaded to the AUR.

## Debian 13 and Ubuntu 24.04

Install the Build-Depends listed in debian/control, then run:

    dpkg-buildpackage -b -us -uc -j2
    sudo apt install ../libfreeimage-maintained3_*.deb ../libfreeimage-maintained-dev_*.deb

Runtime and development files are split into libfreeimage-maintained3 and
libfreeimage-maintained-dev. They conflict with the original libfreeimage3
and libfreeimage-dev packages. They do not satisfy dependencies on those
names: review apt's proposed removals when installing on an existing system.
Use a clean environment for initial testing.

The +packaging1 version identifies this packaging snapshot rather than a
new upstream library release. Maintainer details are placeholders to replace
before publication. The source copyright inventory needs completing before
a Debian archive submission. No PPA or official repository upload is made.

## CI downloads and publication

The Installable Linux packages workflow builds in clean Arch, Debian 13 and
Ubuntu 24.04 containers, installs the packages, runs PNG memory round trips
through both pkg-config and CMake consumers, and checks the SONAME. Download
the arch-x86_64, deb-debian13 or deb-ubuntu24 workflow artifact.

A .deb can be installed with apt locally; it is not yet an apt repository.
PPA publishing requires a Launchpad account, signed source packages and an
upload for each Ubuntu series. A PPA is for Ubuntu, not Debian. Debian users
can use local .deb files or a separately maintained signed apt repository.

Before official replacement of the distribution FreeImage package, compare
exported ABI/symbols and rebuild reverse dependencies. Keeping SONAME 3 here
sets the intended library interface; it does not prove drop-in compatibility.

## Tagged release automation

After this workflow is merged, pushing a numeric version tag such as
3.19.22 builds Linux packages and attaches them to the matching GitHub
release. The tag, checkout, public header and CMake archive fallback must
agree. All three package build/install jobs must pass before publication.

Arch CI builds the exact checkout archive, not the recipe's older pinned
release/backport. Debian and Ubuntu versions have different distribution
suffixes, so their release assets cannot overwrite each other. Pull
requests produce preview package versions and CI artifacts only.

The publishing job creates the matching release with generated notes if
it does not exist; otherwise it preserves its notes and updates Linux
assets. It includes binary packages, an exact source archive, its generated
Arch recipe, and SHA256SUMS-linux.txt. No separate Linux tag is created.

Release the version PR first, tag its merge commit, and push that tag.
This does not create an apt repository or publish to the AUR.
