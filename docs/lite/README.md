# EchoNeubrutalism Lite

Personal EchoPod-based firmware for the **512 MB** Snowsky (FiiO) Echo Mini.

- **UI:** neo-brutalist Lite layout, Light/Dark, 320×170 and 170×320.
- **Hardware:** tested on one 512 MB unit; the 8 GB model is not supported.
- **Current build:** `release-lite` preset; image `MINIV949.IMG`, pending a final device check.
- **Upstream:** EchoPod 0.13.2.
- **Implemented:** All Songs / Favorites roots with no Home; Short Back root switching and
  Long Back Settings; Short Menu as the Now Playing/Up Next key and Long Menu as the screen
  function key; Settings five-card home and three category pages; reusable TrackListScreen with
  Favorites title+duration and A-Z; Now Playing without times or progress thumb; 10×10 status blocks that stay gray until Bluetooth/playback is active; segmented root meter; explicit
  Favorites queue and resume; Pinyin/romaji CJK sorting; singleton overlay navigation; no-title
  All Songs and inset Up Next; portrait-flipped; sleep timer countdown and finish-track shutdown.

## Start here

- [What changed from EchoPod](customizations.md)
- [Design tokens and layout](design.md)
- [CJK sorting and section jump](cjk-sorting.md)
- [Final mockups](mockups/README.md) / [HTML sheet](mockups/index.html)
- [Install and recovery](install-512m.md)
- [Controls](shortcuts.md)

## Build

```sh
cmake --preset release-lite
cmake --build --preset release-lite
```

Output: `build/release-lite/HIFIEC90.IMG`.

Copy it to the card as `MINIVnnn.IMG` for the 512 MB updater. See
[install-512m.md](install-512m.md).

## Scope

The boot chain, IDB, PMU firmware, fw2 and slot layout are not modified. Keep the official
firmware and a recovery path. This is a personal, non-commercial project; see the root README for
the full disclaimer and AI disclosure.
