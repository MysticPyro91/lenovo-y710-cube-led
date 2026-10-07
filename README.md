# lenovo-y710-cube-led

Control the case lights of a **Lenovo IdeaCentre Y710 Cube** (Legion Y710 Cube)
on **Linux**: the red **Y logo** on the front and the **light strips at the
bottom** of the case. On Windows this is done by **Lenovo Nerve Center**; on
Linux there was no way to change or turn off the LEDs until now.

```
$ sudo y710-led get
logo: low (0x0001)
bottom: high (0x1011)
raw: 0x10110007
$ sudo y710-led set off off       # both lights off
$ sudo y710-led set high medium   # logo high, bottom strips medium
```

Each light has four levels: `off`, `low`, `medium`, `high`. The hardware has no
colour or breathing/pulse modes.

## Supported hardware

| Model | Machine type | BIOS | Status |
| --- | --- | --- | --- |
| IdeaCentre Y710 Cube-15ISH | 90FL | O2DKT17A | Tested: both lights, all levels |

Other Y710 Cube variants and BIOS versions are untested but likely work the
same way, since Nerve Center uses one LED driver for the model.
If yours works or doesn't, please open an issue with the output of
`sudo y710-led get` and `cat /sys/class/dmi/id/{product_name,product_version,bios_version}`.

The tool checks DMI and refuses to run on anything that does not report itself
as a Lenovo Y710 Cube. It is **not** for Legion laptops (Y520, Y530, Y540,
Y720, Y740, Legion 5/7 and others); those use different controllers, see for example
[LenovoLegionLinux](https://github.com/johnfanv2/LenovoLegionLinux).

## Install

Arch Linux (the AUR package is not up yet; build the package from this repo):

```
git clone https://github.com/MysticPyro91/lenovo-y710-cube-led
cd lenovo-y710-cube-led/aur
makepkg -si
```

From source (needs `gcc` and `make`):

```
git clone https://github.com/MysticPyro91/lenovo-y710-cube-led
cd lenovo-y710-cube-led
make
sudo make install          # installs /usr/local/bin/y710-led
```

## Usage

```
y710-led [--force] get
y710-led [--force] set LOGO BOTTOM
y710-led --help | --version
```

- `get` reads both lights and changes nothing.
- `set LOGO BOTTOM` sets the Y logo and the bottom strips.
- Root is required (it talks to the firmware through I/O port 0xB2).
- `--force` skips the DMI model check. Only use it on a real Y710 Cube whose DMI
  data is missing or odd.

Turn the lights off at every boot with a small systemd unit, for example
`/etc/systemd/system/y710-led-off.service`:

```
[Unit]
Description=Turn off the Y710 Cube case lights

[Service]
Type=oneshot
ExecStart=/usr/bin/y710-led set off off

[Install]
WantedBy=multi-user.target
```

(Use `/usr/local/bin/y710-led` if you installed from source.) Then
`sudo systemctl enable y710-led-off.service`.

## How it works

The lights are switched by the BIOS through a System Management Interrupt:
write `EAX=0x0B` to port `0xB2` with `EBX=0x10` (set, light value in `ECX`) or
`EBX=0x11` (get). The values were worked out by analysing what Lenovo's Windows
LED driver sends; no Lenovo code is included. Details: [PROTOCOL.md](PROTOCOL.md).

## Safety

- Only the two documented LED subcommands (`0x10` set, `0x11` get) are ever sent.
- The process pins itself to CPU 0, asks only for port `0xB2`, and drops the port
  permission when done.
- Firmware error bits are reported and give a non-zero exit code.
- Kernel lockdown (`integrity`/`confidentiality`, e.g. with Secure Boot on some
  distros) blocks raw port access; the tool then fails with "cannot access port
  0xB2" and changes nothing.

It has worked fine on the tested machine, but it pokes firmware directly, so use
it at your own risk (see [LICENSE](LICENSE)).

## Keywords

Lenovo IdeaCentre Y710 Cube, Legion Y710 Cube, Y710 Cube-15ISH, 90FL,
Nerve Center Linux, Lenovo case LED Linux, turn off LED lights Y710 Cube,
yled.sys, Lenovo gaming desktop lighting, Arch Linux, Ubuntu, Fedora.

## License

MIT, see [LICENSE](LICENSE).
