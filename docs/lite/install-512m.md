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
build/release-lite/HIFIEC90.IMG  ->  /MINIV938.IMG
```

Then use the player's updater.

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

## Current build

```text
image:  MINIV938.IMG
sha256: ebf7c3c2c97a6fb78933fbe3c595ba35be9b34ee5bf28afb8d8926623f105fee
```

`MINIV938` is the final incremental UI build for this pass. It contains the runtime Light/Dark
and landscape/portrait/portrait-flipped modes, the 8 px padding system, the 28×8 global status
meter, the 112×112 portrait Home card, the landscape Settings screen without a text header,
black dark-mode shadows, and 200×112 / 146×112 boot stills with 5 px hard shadows and no card
or frame. The navigation is the singleton-overlay model: Back returns to the opener, Now
Playing / Quick Panel / Settings are not duplicated, and long Play no longer opens the panel.
Menu is always Up Next in Lite; double Menu opens the Quick Panel in Now Playing, the next
category in Settings, and the A-Z/# overlay in lettered lists. All Songs has no title, Up Next
uses the inset card in landscape, and page / overlay / category motion follows the layered
motion design. CJK titles are sorted by pinyin (GB2312) and kana by romaji, interleaved with
Latin under A-Z; the boot still is drawn across the display strips.

It still needs a final device check; earlier Lite builds were tested on one 512 MB unit.
The incremental images for this pass are:

| Image | Contents | SHA-256 |
|---|---|---|
| `MINIV935` | singleton-overlay navigation, Menu behavior, double-Menu infrastructure | `d5614b05560c26578e8486f008b8693dfaca339d237b93751457c1aab84b3ba2` |
| `MINIV936` | section initial fix, A-Z/# overlay, volume-hold cleanup, All Songs / Up Next headers | `5a2dbdf7f0ea4a8920019a398b818297597bf723f214944e5ac0ccd94c2b6d47` |
| `MINIV937` | page/overlay motion, Settings category transition, portrait-flipped | `4e4600bec521765a5b32986db06447a9bede7b2e582ccc40f03671499dcc3f4c` |
| `MINIV938` | final fixes: Menu closes the A-Z overlay, Up Next row sizing, compatibility initials and docs | `ebf7c3c2c97a6fb78933fbe3c595ba35be9b34ee5bf28afb8d8926623f105fee` |

The 8 GB model is not supported.

## Verification

- Host tests: 168/168 passed (`ctest --preset dev -E "^format$"`).
- Lite panel tests: 51/51 passed.
- `release-lite` build and updater check valid.
- Still pending: the final 512 MB device pass over sorting, boot, dots, headers, navigation,
  motion and portrait-flipped.

## After flashing

The updater needs a power cycle. If the player does not start, use the official recovery path
instead of repeatedly flashing. Do not flash an 8 GB image or a stock image on a 512 MB player.
