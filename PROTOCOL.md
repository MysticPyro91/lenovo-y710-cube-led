# Y710 Cube LED protocol

How the case lights of the Lenovo IdeaCentre Y710 Cube are controlled, as
found by static analysis of Lenovo Nerve Center's LED driver (`yled.sys`) and
its managed front end (`GameZone.Manager.dll`) for interoperability. No Lenovo
code or binaries are part of this repository.

## The call

Nerve Center's kernel driver triggers a System Management Interrupt (SMI) by
writing to the APM control port:

| Register | Value |
| --- | --- |
| DX | `0xB2` (port) |
| EAX | `0x0B` (LED command; AL is the byte written to the port) |
| EBX | `0x10` = set, `0x11` = get |
| ECX | set: the light value below; get: returns both lights |

After the SMI, `EAX & 0xC0000000` set means the firmware reports an error.

This is a different SMI command from the BIOS-settings WMI interface in the
DSDT (`SMIK`, command `0xEF`); `y710-led` never uses that one.

## Values

One set call per light:

| Level | Light 1: Y logo | Light 2: bottom strips |
| --- | --- | --- |
| off | `0x0000` | `0x1000` |
| low | `0x0001` | `0x1001` |
| medium | `0x0010` | `0x1010` |
| high | `0x0011` | `0x1011` |

A get call returns both in ECX: light 1 is `ECX & 0x1111`, light 2 is
`ECX >> 16`. That is the same decoding Nerve Center uses. On the tested machine
the low half also had other bits set (raw `0x10110007`); their meaning is
unknown and they are masked out.

## Not used

- `EBX=0x01` ("load default") exists in the driver but is called with an
  uninitialised ECX; it is never sent here.
- There are no breathing, pulse or colour modes on this model.

## Notes

- The SMI is issued from CPU 0 (the tool pins itself there).
- Whether the firmware keeps the setting across a full power cycle is not
  verified yet.
