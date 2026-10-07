# Overview

`y710-led` sets the two case lights of a Lenovo IdeaCentre Y710 Cube (the Y logo
and the bottom light strips) from Linux, replacing Lenovo Nerve Center.

## Layout

| Path | What |
| --- | --- |
| `y710-led.c` | The whole tool: DMI check, CPU 0 pinning, `ioperm(0xB2)`, SMI call, value encoding |
| `Makefile` | `make`, `make install` (`PREFIX`, `DESTDIR`), `make uninstall` |
| `PROTOCOL.md` | The SMI interface: registers, subcommands, value bits |
| `aur/PKGBUILD` | Arch package, built from the GitHub release tarball |
| `CHANGELOG.md` | Release notes |

## Release

1. Update `CHANGELOG.md`, bump `VERSION` in `y710-led.c` and `pkgver` in `aur/PKGBUILD`.
2. Tag `vX.Y.Z`, push the tag, create the GitHub release.
3. In `aur/`: `updpkgsums`, `makepkg --printsrcinfo > .SRCINFO`, `makepkg -f` as a build check.
4. Copy `PKGBUILD` and `.SRCINFO` into the AUR git clone (`ssh://aur@aur.archlinux.org/lenovo-y710-cube-led.git`), commit, push.

## Tested hardware

IdeaCentre Y710 Cube-15ISH, type 90FL, BIOS O2DKT17A. New reports go into the
README table.
