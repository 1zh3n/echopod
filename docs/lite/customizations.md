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
- No Home screen. All Songs and Favorites are roots; Short Back at the root switches them.
  The deferred root-tab icons are not drawn yet, so the existing list frames stay unchanged.
- Song Favorites: a path-hashed list of up to 256 tracks, shown like All Songs (title + duration,
  A-Z jump), played as its own queue, with Add/Remove in the song actions menu and direct removal
  from the Favorites list. The list is built through a reusable TrackListScreen for future
  explicit track collections.
- 28×8 global status meter drawn inside each screen: blue = Bluetooth, cyan = playing; Now
  Playing uses a 14×8 Bluetooth-only chip beside shuffle/play. The meter moves with the page.
- Settings opens on five cards: Random mode, Sleep timer, Sound, Display, System. Random and
  Sleep are direct items; the other three open category pages. Category dots are removed.
- Settings is reached by Long Back; Long Menu is the screen's function key and never opens
  Settings. Inside Settings, Long Back returns to the current root.
- Landscape Settings has no text header; the half-badge, glyph and rows carry the
  category. Portrait keeps the category name.
- Settings category pages keep the half-badge, glyph and colour; rows stay static. Page slides
  keep their existing motion.
- Now Playing uses native 18 px mode/state icons, no cover frame, and a Bluetooth-only chip.
  The current/total time row and progress thumb are removed; only the volume number appears
  temporarily while changing volume.
- Boot uses 200×112 landscape / 146×112 portrait stills with a 5px hard shadow, no card or frame.
- Charging keeps the 96×64 animation card.
- Runtime Light/Dark and Landscape/Portrait/Portrait-flipped in Settings → Display.
- Dark borders: large surfaces 1 px `#52525B`, badges 2 px light, selection blue; shadows are
  `#000000` so they read as shadows against the dark page.

## Controls

- Now Playing: volume single press = volume, double press = previous/next, long press = seek.
- Boot goes directly to All Songs. Short Back at the root switches All Songs <-> Favorites;
  deeper Short Back goes up one level.
- Short Menu opens Now Playing; on Now Playing it opens Up Next. There is no double Menu.
- Long Menu is the current screen's function: A-Z in All Songs/Favorites, next row in A-Z,
  current song actions in Now Playing, no action in Up Next/Settings.
- Long Back opens Settings from a root, a list, Now Playing or A-Z; inside Settings it returns
  to the current root.
- Long Play on a track opens Play Next / Add to Up Next / Add or Remove Favorites. It is a no-op
  in Now Playing and Settings. Long Play on a Favorites row removes that song.
- In the A-Z/# overlay, short Back or Menu closes it; long Back closes it to the root.
- Volume hold only scrolls; section jump moved to long Menu.
- The Lite Menu-key setting is hidden; Short Menu is the Now Playing/Up Next navigation key.

## Code and assets

- Song favorites use `/.echo/favorite_tracks.bin`; album favorites stay available to non-Lite.
- The Favorites queue is an explicit track list in `PlayQueue` and is saved in the resume file,
  so next/previous and restart stay inside Favorites.
- Lite icon masks moved from the code window to a glyph face (`assets/lite_icons.bin`). The
  Favorites heart mask is kept for the deferred root tabs; the existing list frames do not draw
  it yet.
- Lite theme store/assets removed; custom card themes are not loaded in Lite.
- Portrait boot still added as `LBO2` in `assets/lite_animation.bin`.
- Boot art: Mountain Dusk by ansimuz, CC0 1.0.
- Lite icons and badges are drawn procedurally.

## Not changed

- Boot chain, IDB, PMU firmware, fw2 and slot layout.
- The generic EchoPod feature set remains in the source for reference and for non-Lite builds.
- Library rescan performance is a separate task.
