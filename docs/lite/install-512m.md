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
build/release-lite/HIFIEC90.IMG  ->  /MINIV945.IMG
```

Then use the player's updater.

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

## Current build

```text
image:  MINIV945.IMG
sha256: 0f6ee445ae536dc581bf88b4702e1d142666427e5332f7f78b0256164dc65f67
```

`MINIV945` replaces the status meter/chip with flat 10×10 blocks: Bluetooth/play on the
right, All Songs/Favorites markers on the left of root lists, gray inactive and coloured active,
aligned at y=8. The Now Playing repeat/loop-one masks were redrawn on the 18px grid.

`MINIV944` unifies the Menu keys: Short Menu always goes to Now Playing (then Up Next from Now
Playing), while Long Menu is the screen function key — A-Z in All Songs/Favorites, next row in the
A-Z overlay, current-song actions in Now Playing, and no action in Up Next/Settings. Long Back
opens Settings; inside Settings it returns to the current root. Settings opens on five cards:
Random mode, Sleep timer, Sound, Display, System. Category dots are removed. Favorites now looks
like All Songs (title + duration, A-Z) through a reusable track-list screen. Now Playing removes
the time row and progress thumb but keeps the transient volume number.

It also keeps the Favorites collection cache between root switches. Boot goes directly to All
Songs; Short Back at the root switches to Favorites and back, and Long Back opens Settings. The root-tab icons are deferred,
so the list frames and rows keep their existing appearance. Favorites is a path-hashed song list
of up to 256 tracks that plays as its own queue and restores through the resume file. Long Play
on a track in All Songs, albums, folders, playlists or Up Next opens Play Next / Add to Up Next /
Add or Remove Favorites. Settings opens on five cards — Random mode, Sleep timer, Sound,
Display, System — with no category dots; the status blocks are drawn inside each screen so they move
with the page. The build also keeps the runtime Light/Dark and
landscape/portrait/portrait-flipped modes, the 8 px padding system, the 28×8 status meter, the
existing list frames, no-text landscape Settings, black dark-mode shadows, and 200×112 / 146×112
boot stills. CJK titles sort by pinyin/romaji and stay interleaved under A-Z.

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
| `MINIV941` | song Favorites on Home, explicit favorites queue and resume, Add/Remove actions; audio-fed collection and Lite-only actions | `d1eb274b6fa84ae431c0aca12c9d3d13b59f67a771b02f1c011cbe4024a019c2` |
| `MINIV942` | remove Home; All Songs / Favorites roots, root Back switching, Long Back Settings; Settings Back returns the root; root tabs deferred | `e3baece2bcd554eb401c837bbe7238037e475d4d39e9ae8b264d8a0be316eecc` |
| `MINIV943` | cache the Favorites collection between root switches; stop scanning after the last favorite | `511d1e9d659d09e0f5b11e7bca01ea09da395e579627de334ed2d5755f828721` |
| `MINIV944` | unified Menu keys, Settings five-card home, Favorites A-Z/duration, Now Playing cleanup | `0f6ee445ae536dc581bf88b4702e1d142666427e5332f7f78b0256164dc65f67` |

The 8 GB model is not supported.

## Verification

- Host tests: 168/168 passed (`ctest --preset dev -E "^format$"`).
- Lite panel tests: 74/74 passed.
- `release-lite` build and updater check valid.
- Headroom: code 136,472 / 157,292 bytes (20,820 free); loaded total 409,152 / 437,600 bytes
  (28,448 free); resident tables 18,236 / 18,432 (196 free); rescan overlay 85,400 / 86,016
  (616 free).
- Still pending: the final 512 MB device pass over sorting, boot, dots, headers, navigation,
  local motion, root switching, Settings cards, favorites and portrait-flipped.

## After flashing

The updater needs a power cycle. If the player does not start, use the official recovery path
instead of repeatedly flashing. Do not flash an 8 GB image or a stock image on a 512 MB player.
