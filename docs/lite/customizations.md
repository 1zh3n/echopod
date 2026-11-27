# What changed from EchoPod

This fork keeps EchoPod's playback engine and hardware support, and adds a personal 512 MB Lite
front end. The list below is the useful summary; the git history has the exact changes.

## Platform

- 512 MB (SD NAND) port and build configuration.
- Lite build (`ECHO_LITE`), no cover cache, no non-Lite diagnostics.
- `release-lite` CMake preset.
- Upstream EchoPod 0.13.2 playback, USB, FAT and HE-AAC fixes included.

## UI

- Neo-brutalist Lite visual language: 2 px borders, hard shadows, flat colour badges.
- Three-icon Home carousel: All Songs / Playlists / Settings, no text; 6 px card radius.
- 28×8 global status meter: blue = Bluetooth, cyan = playing; Now Playing uses a 14×8
  Bluetooth-only chip beside shuffle/play.
- Settings is one category per screen: Sound / Display / Library / Buttons / System.
  Double Menu moves to the next category; long Play no longer does.
- Landscape Settings has no text header; the half-badge, category dots and rows carry the
  category. Portrait keeps the category name.
- Settings category changes use a short horizontal content transition; Home and list selection
  keep the motion rules from the motion spec.
- Quick panel with two icon-only cards: playback mode and sleep timer.
- Now Playing uses native 18 px mode/state icons, no cover frame, and a Bluetooth-only chip.
- Boot uses 200×112 landscape / 146×112 portrait stills with a 5px hard shadow, no card or frame.
- Charging keeps the 96×64 animation card.
- Runtime Light/Dark and Landscape/Portrait/Portrait-flipped in Settings → Display.
- Dark borders: large surfaces 1 px `#52525B`, badges 2 px light, selection blue; shadows are
  `#000000` so they read as shadows against the dark page.

## Controls

- Now Playing: volume single press = volume, double press = previous/next, long press = seek.
- Double Menu in Now Playing opens the playback panel; long Play is a no-op there.
- Long Menu opens Settings everywhere; in Settings it is a no-op.
- Long Back in the playback panel goes Home.
- Lists use long Play only where a context action exists; double Menu opens the A-Z/# jump.
- The Lite Menu-key setting is hidden; Menu always opens Up Next.
- Volume hold only scrolls; section jump moved to double Menu.

## Code and assets

- Lite icon masks moved from the code window to a glyph face (`assets/lite_icons.bin`).
- Lite theme store/assets removed; custom card themes are not loaded in Lite.
- Portrait boot still added as `LBO2` in `assets/lite_animation.bin`.
- Boot art: Mountain Dusk by ansimuz, CC0 1.0.
- Lite icons and badges are drawn procedurally.

## Not changed

- Boot chain, IDB, PMU firmware, fw2 and slot layout.
- The generic EchoPod feature set remains in the source for reference and for non-Lite builds.
