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
- Three-icon Home carousel: All Songs / Favorites / Settings, no text; 6 px card radius.
- Song Favorites: a path-hashed list of up to 256 tracks, opened from Home, played as its own
  queue, with Add/Remove in the song actions menu and direct removal from the Favorites list.
- 28×8 global status meter drawn inside each screen: blue = Bluetooth, cyan = playing; Now
  Playing uses a 14×8 Bluetooth-only chip beside shuffle/play. The meter moves with the page.
- Settings is one category per screen: Quick / Sound / Display / System. Long Menu moves to
  the next category; long Play no longer does.
- Settings opens on Quick, with icon-only playback-mode and sleep-timer cards. The standalone
  Quick panel is gone, so Home, Now Playing and the lists no longer need a second overlay.
- Landscape Settings has no text header; the half-badge, category dots and rows carry the
  category. Portrait keeps the category name.
- Settings category changes animate only the badge, glyph and dots; rows stay static. Page
  slides and the Home carousel keep their existing motion.
- Now Playing uses native 18 px mode/state icons, no cover frame, and a Bluetooth-only chip.
- Boot uses 200×112 landscape / 146×112 portrait stills with a 5px hard shadow, no card or frame.
- Charging keeps the 96×64 animation card.
- Runtime Light/Dark and Landscape/Portrait/Portrait-flipped in Settings → Display.
- Dark borders: large surfaces 1 px `#52525B`, badges 2 px light, selection blue; shadows are
  `#000000` so they read as shadows against the dark page.

## Controls

- Now Playing: volume single press = volume, double press = previous/next, long press = seek.
- Menu is always Up Next in Now Playing; long Menu opens Settings. There is no double Menu.
- Long Menu in a lettered list opens the A-Z/# jump; long Menu elsewhere opens Settings.
- Long Play on a track opens Play Next / Add to Up Next / Add or Remove Favorites. It is a no-op
  in Now Playing and Settings. Long Play on a Favorites row removes that song.
- Long Back goes Home. In the A-Z/# overlay, short Back or Menu closes it.
- Volume hold only scrolls; section jump moved to long Menu.
- The Lite Menu-key setting is hidden; Menu always opens Up Next.

## Code and assets

- Song favorites use `/.echo/favorite_tracks.bin`; album favorites stay available to non-Lite.
- The Favorites queue is an explicit track list in `PlayQueue` and is saved in the resume file,
  so next/previous and restart stay inside Favorites.
- Lite icon masks moved from the code window to a glyph face (`assets/lite_icons.bin`); the Home
  Favorites card has its own heart mask instead of reusing the List icon.
- Lite theme store/assets removed; custom card themes are not loaded in Lite.
- Portrait boot still added as `LBO2` in `assets/lite_animation.bin`.
- Boot art: Mountain Dusk by ansimuz, CC0 1.0.
- Lite icons and badges are drawn procedurally.

## Not changed

- Boot chain, IDB, PMU firmware, fw2 and slot layout.
- The generic EchoPod feature set remains in the source for reference and for non-Lite builds.
- Library rescan performance is a separate task.
