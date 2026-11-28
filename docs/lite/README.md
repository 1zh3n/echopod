# EchoNeubrutalism Lite

**Personal, experimental firmware for the 512 MB Snowsky (FiiO) Echo Mini.**

> **Read this before flashing.**
>
> This is my own fork, not an official EchoPod or FiiO release. It is tested on one 512 MB
> unit, not on the 8 GB model. Flashing can brick the player, erase the card or lose data.
> Keep the official firmware and a known recovery path, and do not flash it on hardware you
> cannot recover. There is no warranty.

![Current Lite UI](../images/lite/lite-current.png)

*Current UI in the simulator at 2×: All Songs, Settings, Now Playing and Date/Time. The pixels
come from the firmware itself; colours on a real panel may differ slightly.*

## What this is

This is a personal Lite build of [EchoPod](https://github.com/BasilSkyWalk/echopod) for the
**512 MB** Echo Mini. It keeps EchoPod's playback engine and hardware support and replaces the
front end with a compact neo-brutalist UI for 320×170 and 170×320.

It is not a general replacement firmware. The 8 GB model is not supported.

- **UI:** Lite layout, Light/Dark, landscape/portrait/portrait-flipped.
- **Build:** `release-lite` preset, image size and headroom recorded in
  [install-512m.md](install-512m.md).
- **Upstream:** EchoPod 0.14.0 512 MB support and fixes, plus this fork's Lite changes.
- **Boot chain:** boot chain, IDB, PMU firmware, fw2 and slot layout are not modified.

## Interaction at a glance

There is **no Home screen**. The two roots are **All Songs** and **Favorites**.

```text
Root (All Songs / Favorites)
  ├─ Short Back: switch root
  ├─ Long Back:  Settings
  ├─ Short Menu: Now Playing
  └─ Long Menu:  A-Z jump

Settings
  ├─ Short Back: leave Settings (back to the current root)
  ├─ Long Back:  return to the current root
  └─ Short Menu: Now Playing
```

Inside a list, **Short Back** goes up one level. **Long Back** is always the Settings entry from
a normal screen; inside Settings it returns to the current root. **Short Menu** is navigation
(Now Playing, then Up Next from Now Playing). **Long Menu** is the screen's function key, not a
second navigation key.

### Key map

| Screen | Volume | Play | Menu | Back |
|---|---|---|---|---|
| Root list | move row; hold to scroll faster | short: open | short: Now Playing; long: A-Z | short: switch root; long: Settings |
| Deeper list | move row; hold to scroll faster | short: open; long: song actions on tracks | short: Now Playing; long: A-Z in lettered lists | short: up; long: Settings |
| A-Z / # jump | choose letter | short: jump | short: Now Playing; long: next grid row | short: close; long: Settings |
| Now Playing | short: volume; double: previous/next; long: seek | short: play/pause | short: Up Next; long: current-song actions | short: leave; long: Settings |
| Up Next | move row | short: play | short: Now Playing; long: no action | short: back; long: Settings |
| Settings home | move row | short: change value/open | short: Now Playing; long: no action | short: current root; long: current root |
| Settings category | move row | short: change value/open | short: Now Playing; long: no action | short: Settings home; long: current root |
| Settings subpage | move row | short: change value/open | short: Now Playing; long: no action | short: previous page; long: current root |

Full details and the A-Z grid geometry: [shortcuts.md](shortcuts.md).

### Now Playing

- The time row and progress thumb are removed.
- The progress bar is a flat line; seek is shown by its length. It ends 18 px above the bottom,
  matching the 8 px top margin plus the 10 px status block.
- When the track has no synced lyrics, a small 9×9 ball bounces inside the lyric area. It
  changes colour at each edge and freezes when paused.
- Volume has 85 steps of exactly 1 dB; the default is 30, close to the old level 4.
  While changing volume, the progress bar temporarily turns yellow and follows the volume;
  there is no separate volume number. Old 50-step settings and resume files migrate
  automatically.
- Volume short press changes volume, double press skips, long press seeks.
- Bluetooth, playback and the mode block share the top status row.
- There is no separate playback-mode icon on the page.

### Favorites

Favorites is a real track list, not an album shortcut:

- path-hashed, up to 512 songs;
- title + duration, sorted by title, same A-Z/# jump as All Songs;
- selecting a row starts its **own queue**, so next/previous stay in Favorites;
- Long Play removes the selected song;
- the list is cached between root switches and restored through the resume file.

### Settings and Date/Time

Settings opens on five compact rows:

```text
Random mode    Loop
Sleep timer    Off
Sound          >
Display        >
System         >
```

The rows use five distinct dots (orange, indigo, blue, cyan, purple), right-aligned values and
four visible rows with the next peeking. Random/Sleep are direct; Sound/Display/System open
category pages. Date/Time uses the same card, six fields and an in-card hint; the selected row is
blue and the value turns yellow while editing.

## Screens

| All Songs | Settings |
|---|---|
| ![All Songs](../images/lite/all-songs-current.png) | ![Settings](../images/lite/settings-current.png) |

| Now Playing | Date/Time |
|---|---|
| ![Now Playing](../images/lite/now-playing-current.png) | ![Date/Time](../images/lite/date-time-current.png) |

| Portrait All Songs | Portrait Settings |
|---|---|
| ![Portrait All Songs](../images/lite/all-songs-portrait-current.png) | ![Portrait Settings](../images/lite/settings-portrait-current.png) |

Boot art: [boot-shadow.png](../images/lite/boot-shadow.png). The screenshots are from the simulator,
not device photos; see [design.md](design.md) for tokens and geometry.

## What changed from EchoPod

- **Roots:** All Songs and Favorites; Home removed. Short Back switches roots.
- **Favorites:** song list with its own queue and resume, built through the reusable
  `TrackListScreen`, replacing the old album-only Home shortcut.
- **Settings:** five-row home with colour dots, right-aligned values, four-row scrolling and
  five category pages. Date/Time was moved onto the same card style.
- **Status row:** 10×10 blocks at y=8; Bluetooth, playback, then an always-visible mode block
  (purple random, gray loop, gray `1` loop-one). Inactive states are neutral gray.
- **Volume:** 85 one-decibel steps instead of the old 50 non-linear steps. The default is 30;
  3.5 mm, 4.4 mm and Bluetooth all step by 1 dB, with each output's previous top level kept.
- **Now Playing:** no time row or thumb; when no synced lyrics are available, a small ball
  bounces in the lyric area and changes colour at each edge; progress moved lower into the
  18 px bottom gap and doubles as the temporary volume meter, with no volume number; no
  separate play/pause or mode icon.
- **Sorting:** GB2312 pinyin and kana romaji interleaved with Latin under A-Z. Leading
  articles are literal, so "The End of the World" sorts under T; an existing library needs a
  rescan to pick up the new order.
- **Space work:** the Lite build drops the unused cover cache (about 67 KB of static RAM), so
  song Favorites can hold 512 tracks. Lite app static RAM is 69,586 / 131,072 bytes (61,486
  free); the loaded total has about 95 KB free. Lite icon masks
  moved to the glyph face; Lite theme-store/assets removed. Resident tables still have only
  196 bytes free, so keep new table data small.
- **Review fixes:** Settings categories remember their row; Date/Time preserves an unsaved edit
  across the Now Playing overlay; 90/120-minute sleep countdown formats correctly; dark-mode row
  separators follow the theme; portrait All Songs shows nine rows; list subpages stay below the
  status row.

Detailed diff: [customizations.md](customizations.md). Design: [design.md](design.md).

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

Output: `build/release-lite/HIFIEC90.IMG`.

Host checks:

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -E '^format$'
```

Lite UI tests: `build/lite-test/tests/test_lite_panel`. The full Lite test configuration is not
yet a separate CTest target; treat `test_lite_panel` as the authoritative UI suite.

The Lite app static RAM audit uses the real `release-lite` compile commands:

```sh
python3 tools/lite_ram_audit.py \
  --compile-commands build/release-lite/compile_commands.json \
  --budget-bytes 131072
```

Current result: **69,586 / 131,072 bytes (61,486 free)**. The older `target_audit` compiles the
non-Lite source set, so it is not a substitute for this Lite-specific number.

## Install and recovery

See [install-512m.md](install-512m.md). The 512 MB updater looks for `/ECHOMINI.IMG` or
`/MINIV000.IMG`–`/MINIV999.IMG`, not `HIFIEC90.IMG`. Copy the image to the card with a supported
name, run the updater, and keep the official firmware available.

## Future ideas (not implemented)

### Multiple Favorites lists

The current Favorites root is still a **single** list of up to 512 songs. Multiple lists are
possible, but they are not implemented and are not part of the current image.

If this is picked up later:

- **Data:** either one file per list or one multi-list file with list names/IDs and a key set per
  list. The current `/.echo/favorite_tracks.bin` reservation is only 4 KB; one 512-song list is
  about 2 KB, so 8 lists need roughly 16–20 KB. An existing file would need a reserve/grow or a
  migration path.
- **RAM:** keep only the active list's keys, queue and resume state in RAM; load the other lists
  from the card on demand. Lite currently has 61 KB of app static RAM free, so a small number of
  lists would fit, but loading on demand is simpler.
- **UI/key model:** a list picker needs a key. Long Menu on the Favorites root is currently the
  A-Z jump, so either Long Menu opens the list picker and A-Z moves inside a list, or a small
  `Lists >` row/third root is added. The second option changes the two-root model.
- **Queue/resume:** save the active list ID together with the explicit queue, track and position,
  otherwise switching lists or restarting can restore the wrong queue.

This is a moderate feature, not a hardware or format risk. It was left out because there is no
current need.

### Private audio as a third list

Goal: leave normal All Songs and Favorites untouched, copy recordings into a hidden folder, and
browse and play them only from a third list. This is not implemented. The detailed design,
including the capacity limit and the build options, is in [private-audio.md](private-audio.md).

A folder or an M3U file alone is not enough. The main library indexes every supported audio file,
and an M3U entry has to resolve to a library track. The main scanner already skips dot-prefixed
entries, and the PC indexer skips hidden path parts, so a folder such as `/.private/` is naturally
invisible to the normal library.

Two implementation shapes were considered:

1. **Separate private library (preferred for isolation).** The device builds a small private
   index from `/.private/`, for example `/.echo/private.db`, and the private list uses its own
   `LibraryStore`, track list, queue and resume file. The normal library and Favorites never see
   the private tracks. The device index can reuse the existing rescan overlay with a private scan
   root and a second output; the overlay has about 688 bytes free, so that root change must be
   compiled and measured. A second `LibraryStore` and cache cost roughly 2.5–3 KB of static RAM.
2. **A private section in the normal library.** One player and queue are simpler at runtime, but
   the library format and both indexers need a new section, and every normal query must exclude
   it. The private names remain in `library.db`. This touches the main data format and is more
   invasive than the separate private library.

The private list is **not a third root**. A third root would be reachable by Short Back, and if
private playback continued after leaving it, Short Menu and Up Next would reveal private titles.
Instead it is a **session** opened from a Settings → System → Private audio row, with a
confirmation question. Leaving the list, opening Settings, screen-off/power-off or a timeout
locks and stops the private session and restores normal playback. Normal boot never
auto-resumes the private session.

This is **UI isolation, not encryption**: the files and the private index are still readable over
USB.

**Assessment:** feasible as a medium feature. Device-built scanning is the target and can reuse
the rescan overlay; the open items are the 688-byte overlay margin, the player/library switch and
their RAM/code measurement. It is not scheduled.

### Secret entry and encryption

A game-style key sequence is feasible for hiding or revealing the private session entry. It can
be a simultaneous hold or a sequence detected before normal key dispatch, but it must avoid the
existing long-press meanings (Long Back is Settings, Long Menu is A-Z, volume hold is seek or
scroll). The code cost is small and no storage is needed. It is a convenience, not a secret: the
sequence is in the firmware and is visible to anyone watching or reading the source.

Real encryption is a different project:

- A fixed key sequence has little entropy and is a weak password; a longer sequence is awkward
  with five keys and can still be shoulder-surfed or brute-forced.
- The player has no secure element. A key stored in internal flash or RAM can be extracted, and
  encrypted files can still be copied off the card.
- Decrypting every codec's file stream on the Cortex-M3 adds audio-path work, code and a key-entry
  UI; it needs its own performance and flash/security review.

For a personal player, the practical boundary is: a hidden private folder and an optional secret
entry for casual separation; no claim of cryptographic privacy. A true encrypted vault remains
out of scope.

**Assessment:** a secret key sequence for the private session is feasible and cheap; a fixed
sequence is not encryption. A genuine encrypted vault would need a separate security/audio
project, not a hidden-key trick.

## Scope and caution

- **512 MB only.** The 8 GB model is not supported.
- **Personal project.** Not affiliated with or supported by FiiO, Snowsky or EchoPod upstream.
- **No signature check is assumed.** Flashing modified firmware is at your own risk.
- **One device tested.** The final device pass is still pending for the newest build.
- **AI-assisted.** The Lite changes, tests and docs were generated with AI under human direction;
  see [AI_DISCLOSURE.md](../../AI_DISCLOSURE.md).
- **Not changed:** boot chain, IDB, PMU firmware, fw2 and slot layout.
