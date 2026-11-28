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
build/release-lite/HIFIEC90.IMG  ->  /MINIV960.IMG
```

Then use the player's updater.

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

## Current build

```text
image:  MINIV960.IMG
sha256: 9ebc29a3cd3f620794bdc8847c906c555ecb37c3e8002fddb6b93c9ec3de75fd
```

`MINIV960` keeps the 512-track Favorites/queue/resume format and also reads the older
558-byte Lite resume file, so a resume saved before the capacity change is not lost; the next
save writes the larger file.

`MINIV959` syncs upstream EchoPod 0.14.0 and its 512 MB fixes, removes the unused Lite cover
cache (about 67 KB of static RAM) and raises the Lite song-favorites limit from 256 to 512.
The eMMC/SD-NAND start-up, updater and power-off changes come from upstream. An existing
library still needs a rescan for the literal sorting.

`MINIV958` sorts leading articles literally: "The End of the World" now falls under T, not E.
The C sort key and the PC indexer both changed; an existing library needs a rescan. An
existing card's library order stays old until that rescan.

`MINIV957` gives the five Settings rows distinct dot colours: orange, indigo, blue, cyan and
purple, so adjacent rows no longer share a blue.

`MINIV956` fixes the review findings: list subpages (Up Next, Folders, Bluetooth) stay below
the global status row; Settings categories remember their selected row; Date/Time keeps an
unsaved edit across the Now Playing overlay; the 90/120-minute sleep countdown formats minutes
correctly; dark-mode row separators use the theme divider; and portrait All Songs shows nine
rows.

`MINIV955` fixes the playback-mode cycle when no track is loaded: a stale shuffled queue plan
no longer makes Random mode look stuck. It also puts Date/Time in the same card layout as the
Settings category pages: six rows, a blue selection row, a yellow value while editing, and the
hint inside the card.

`MINIV954` adds clear gaps between the compact Settings rows: 304×28 landscape / 154×52
portrait, 4 px / 6 px gaps, 8 px colour dots, and four rows visible plus the next peeking.

`MINIV953` makes the Settings home compact with colour dots, four rows visible plus the next
peeking, and direct values right-aligned. The last System row no longer clips.

`MINIV952` keeps the mode block right of playback and always visible: random is purple with no
character, loop is gray with no character, and loop one is gray with a centered `1`.

`MINIV951` puts the mode block immediately right of the playback block, so the top-right group
is `[Bluetooth] [play] [mode]`.

`MINIV950` doubles the Settings home cards to 304×50 landscape / 154×96 portrait and lets the
list scroll two cards at a time. The playback mode moves into the status bar as a purple 10×10
block showing `L`, `1` or `S`, only while playing; the separate 18 px mode icon is gone. The Now
Playing spectrum and progress bar move down clear of the album line.

`MINIV949` makes the right status blocks neutral gray when Bluetooth/playback are off, so
colour only appears for an active state, and removes Now Playing's persistent play/pause icon.

`MINIV948` enlarges the Settings home cards with 18/20 px colour icons and title-weight labels,
and separates the right status colours: Bluetooth `#0F8ADB`, playing `#F59E0B`.

`MINIV947` restores the original segmented root meter on the left: 28×10, rounded 2px
border, 2px shadow, two 12×6 cells; the active root is bright, the other is divider gray.

`MINIV946` gives the status blocks their own muted colours and 1px shadows, restores the
segmented All Songs/Favorites rectangle on root lists, and moves Now Playing's title/icons down
to share the same top status row. Long Menu A-Z is now available on Favorites even with one
favorite.

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
of up to 512 tracks that plays as its own queue and restores through the resume file. Long Play
on a track in All Songs, albums, folders, playlists or Up Next opens Play Next / Add to Up Next /
Add or Remove Favorites. Settings opens on five cards — Random mode, Sleep timer, Sound,
Display, System — with no category dots; the status blocks are drawn inside each screen so they move
with the page. The build also keeps the runtime Light/Dark and
landscape/portrait/portrait-flipped modes, the 8 px padding system, the status blocks, the
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
| `MINIV944` | unified Menu keys, Settings five-card home, Favorites A-Z/duration, Now Playing cleanup | `d5acfe888aeb812d55f18cf85c66838d376e8690cc3c54e0504ba9ee7559bf20` |
| `MINIV945` | flat 10×10 status blocks, root markers, cleaner repeat icon | `0f6ee445ae536dc581bf88b4702e1d142666427e5332f7f78b0256164dc65f67` |
| `MINIV946` | coloured shadowed blocks, global Now Playing status row, Favorites A-Z threshold removed | `e5bc90b5270c36686b2dc977196708abee84cd26cfac281ccf1d0abaeac313c5` |
| `MINIV947` | original rounded segmented root meter on the left | `0736cf5f1c8956c8403ccac23786fbd020606609662744a1620eabada67903e0` |
| `MINIV948` | larger Settings cards with icons, Bluetooth blue / playing amber | `4914db5e1ea1f4205f962939d05cfdf305e8efbb0e137c81032abcffb207101b` |
| `MINIV949` | gray status blocks when off, no persistent Now Playing play/pause icon | `0a48f8ac5b9c67d44a8c464fb2ae41cdf51729a2e3dbcd6795252f90847cb434` |
| `MINIV950` | double-height scrolling Settings cards, `L`/`1`/`S` mode block, lower Now Playing spectrum | `1fe93f7ce39bc760a8276f1ba2003b639ca080633104d1b46386964b1b2e7aec` |
| `MINIV951` | mode block immediately right of playback | `c31bc36b6073ab3e38c07a2c536e0e009423922672a62d314b006b832030230a` |
| `MINIV952` | lower Settings cards, right-aligned values, always-visible mode block | `4cb75dc860ea517d95f0d0362ebdad6fc77ebd302f05d58672027af768faff16` |
| `MINIV953` | compact Settings rows with colour dots, four visible | `01e4401a6e9c54420ce687676bc33582b2bbd43cd42cc660c1e9e1f90c94ad37` |
| `MINIV954` | clearer Settings row spacing, 8 px dots, four visible | `f29398dd249ac3c1c8f2e29debebf6964bd8ae2727097b7755cbb8e5d9773aa7` |
| `MINIV955` | Random mode cycle fix, Date/Time card layout, yellow edit value | `1ab6ae94dabc402f07b3974571bd26f96e813f9f882741d50bb0f6425d7fe798` |
| `MINIV956` | list subpage inset, category selection memory, Date/Time edit preservation, sleep countdown and dark divider fixes | `7bddbdc05f4d46825871a90be24b261fda08061359365502a7d44549e1d55dd4` |
| `MINIV957` | five distinct Settings dot colours | `cd58010c7c586fa8c8c3b7d2a99897be13368849f57f3e63a76dd1e55834add3` |
| `MINIV958` | literal leading-article sorting; "The ..." under T | `de625aedc403db1ba5aead8603efa7bbc289d5d21b2f7c86163637af5d68e534` |
| `MINIV959` | upstream 0.14.0/512 MB fixes, Lite cover cache removed, 512 favorites | `84ca0de4501bbc858bdad3aa457cb97e40759211a6e221e03902bc6bc95b0988` |
| `MINIV960` | read old 558-byte Lite resume files | `9ebc29a3cd3f620794bdc8847c906c555ecb37c3e8002fddb6b93c9ec3de75fd` |

The 8 GB model is not supported.

## Verification

- Host tests: 168/168 passed (`ctest --preset dev -E "^format$"`).
- Lite panel tests: 82/82 passed.
- `release-lite` build and updater check valid.
- Headroom: code 136,740 / 157,292 bytes (20,552 free); loaded total 342,108 / 437,600 bytes
  (95,492 free); resident tables 18,236 / 18,432 (196 free); rescan overlay 85,328 / 86,016
  (688 free).
- Still pending: the final 512 MB device pass over sorting, boot, dots, headers, navigation,
  local motion, root switching, Settings cards, favorites and portrait-flipped.

## After flashing

The updater needs a power cycle. If the player does not start, use the official recovery path
instead of repeatedly flashing. Do not flash an 8 GB image or a stock image on a 512 MB player.
