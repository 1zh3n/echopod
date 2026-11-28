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
build/release-lite/HIFIEC90.IMG  ->  /MINIV941.IMG
```

Then use the player's updater.

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

## Current build

```text
image:  MINIV941.IMG
sha256: e5606ffea6494eb58675ee8d18c8d1ab6ef89351dac2de42089d7d4404944f7b
```

`MINIV941` is the song-favorites build. Lite Home is All Songs / Favorites / Settings; Favorites
is a path-hashed song list of up to 256 tracks that plays as its own queue and restores through
the resume file. Long Play on a track in All Songs, albums, folders, playlists or Up Next opens
Play Next / Add to Up Next / Add or Remove Favorites. Settings absorbed the old Quick panel:
Quick / Sound / Display / System, 400 ms long Menu for the next category or Settings, and no
double Menu. Category changes animate only the badge, glyph and dots; the status meter is drawn
inside each screen so it moves with the page. The build also keeps the runtime Light/Dark and
landscape/portrait/portrait-flipped modes, the 8 px padding system, the 28×8 status meter, the
112×112 portrait Home card, no-text landscape Settings, black dark-mode shadows, and 200×112 /
146×112 boot stills. CJK titles sort by pinyin/romaji and stay interleaved under A-Z.

It still needs a final device check; earlier Lite builds were tested on one 512 MB unit.
The incremental images for this pass are:

| Image | Contents | SHA-256 |
|---|---|---|
| `MINIV935` | singleton-overlay navigation, Menu behavior, double-Menu infrastructure | `d5614b05560c26578e8486f008b8693dfaca339d237b93751457c1aab84b3ba2` |
| `MINIV936` | section initial fix, A-Z/# overlay, volume-hold cleanup, All Songs / Up Next headers | `5a2dbdf7f0ea4a8920019a398b818297597bf723f214944e5ac0ccd94c2b6d47` |
| `MINIV937` | page/overlay motion, Settings category transition, portrait-flipped | `4e4600bec521765a5b32986db06447a9bede7b2e582ccc40f03671499dcc3f4c` |
| `MINIV938` | final fixes: Menu closes the A-Z overlay, Up Next row sizing, compatibility initials and docs | `6c59b5d64a8b8b42a89298f8b5d67226e16d231f230408688ce56bbd703355b0` |
| `MINIV939` | Settings absorbs Quick; 400 ms long Menu; standalone Quick panel removed | `962a349f7a2ca78c6a3775a7aea11ae152e94f8138fdca458dc086cab2ee9e0d` |
| `MINIV940` | local motion, screen-owned status meter, static Settings rows | `d9f06accd78d268e375eac79c656c52421b0a1323f7679af34cc5fb6fe9b112d` |
| `MINIV941` | song Favorites on Home, explicit favorites queue and resume, Add/Remove actions | `e5606ffea6494eb58675ee8d18c8d1ab6ef89351dac2de42089d7d4404944f7b` |

The 8 GB model is not supported.

## Verification

- Host tests: 168/168 passed (`ctest --preset dev -E "^format$"`).
- Lite panel tests: 57/57 passed.
- `release-lite` build and updater check valid.
- Headroom: code 137,980 / 157,292 bytes (19,312 free); loaded total 410,524 / 437,600 bytes
  (27,076 free); resident tables 18,240 / 18,432 (192 free); rescan overlay 85,400 / 86,016
  (616 free).
- Still pending: the final 512 MB device pass over sorting, boot, dots, headers, navigation,
  local motion, favorites and portrait-flipped.

## After flashing

The updater needs a power cycle. If the player does not start, use the official recovery path
instead of repeatedly flashing. Do not flash an 8 GB image or a stock image on a 512 MB player.
