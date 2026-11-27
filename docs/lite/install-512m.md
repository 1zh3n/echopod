# Install on the 512 MB Echo Mini

This fork is for the **512 MB** Snowsky (FiiO) Echo Mini only. The 8 GB model is not supported.

## Before flashing

- Keep a copy of the official 512 MB firmware.
- Know the recovery path: official updater or MaskROM; read `../device-install.md` and
  `../device-recovery.md`.
- Flashing custom firmware can brick the device or lose data. See the root README disclaimer.

## Image name

The 512 MB updater scans the card root for:

```text
/ECHOMINI.IMG
/MINIV000.IMG ... /MINIV999.IMG
```

It does **not** scan `HIFIEC90.IMG`. Copy the build to the card root and rename it, for example:

```text
build/release-lite/HIFIEC90.IMG  ->  /MINIV926.IMG
```

Then use the player's updater.

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

## Current build

```text
image:  MINIV926.IMG
sha256: 9a11dc6ff8fe5b873b8b17d55546a4b3945ae0ed9a75e17969e5e3d6b6f2ac24
```

`MINIV926` contains the runtime Light/Dark and landscape/portrait build, the 8 px padding system,
the 28×8 global status meter at (8, 4) with a 14×8 Bluetooth-only chip on Now Playing, the
112×112 portrait Home card, the landscape Settings screen without a text header, black
dark-mode shadows, and card-sized boot stills. It still needs a final device check; earlier Lite
builds were tested on one 512 MB unit.

## After flashing

The updater needs a power cycle. If the player does not start, use the official recovery path
instead of repeatedly flashing. Do not flash an 8 GB image or a stock image on a 512 MB player.
