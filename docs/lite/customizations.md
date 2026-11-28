# What changed from EchoPod

This fork keeps EchoPod's playback engine and hardware support, and adds a personal 512 MB Lite
front end. The list below is the useful summary; the git history has the exact changes.

## Platform

- 512 MB (SD NAND) port and build configuration.
- Lite build (`ECHO_LITE`), no cover cache, no non-Lite diagnostics.
- `release-lite` CMake preset.
- Upstream EchoPod 0.14.0 512 MB support, playback, USB, FAT and HE-AAC fixes included.

## UI

- Neo-brutalist Lite visual language: 2 px borders, hard shadows, flat colour badges.
- No Home screen. All Songs and Favorites are roots; Short Back at the root switches them.
  The deferred root-tab icons are not drawn yet, so the existing list frames stay unchanged.
- Song Favorites: a path-hashed list of up to 512 tracks, shown like All Songs (title + duration,
  A-Z jump), played as its own queue, with Add/Remove in the song actions menu and direct removal
  from the Favorites list. The list is built through a reusable TrackListScreen for future
  explicit track collections.
- 10×10 status blocks with 1 px shadows: Bluetooth `#0F8ADB`, playing `#F59E0B`, then the mode
  block, all on the right; `text_tertiary` gray when off. The mode block is always shown: random
  is purple with no glyph, loop is gray with no glyph, loop one is gray with a white `1`. Root
  lists use the original 28×10 segmented meter on the left; the active half is bright, the other
  divider gray. Now Playing uses the same top row with no separate mode icon.
- Settings opens on five compact rows with an 8 px colour dot on the left: Random mode, Sleep
  timer, Sound, Display, System. The dots are orange, indigo, blue, cyan and purple in row order. Random and Sleep are direct items with their values right-aligned;
  the other three open category pages. The home scrolls four rows at a time with the next row
  peeking. Category dots in the category pages are removed.
- Date/Time uses the Settings card with six rows and an in-card hint; the selected row is blue
  and the value turns yellow while editing.
- Settings is reached by Long Back; Long Menu is the screen's function key and never opens
  Settings. Inside Settings, Long Back returns to the current root.
- Landscape Settings has no text header; the half-badge, glyph and rows carry the
  category. Portrait keeps the category name.
- Settings category pages keep the half-badge, glyph and colour; rows stay static. Page slides
  keep their existing motion.
- Now Playing has no separate playback-mode icon or persistent play/pause icon: the amber status
  block carries the playing state and the mode block carries random/loop/loop-one. The spectrum
  and progress bar sit lower, clear of the album line.
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
  The Lite build removes the unused cover cache and raises the Favorites/queue/resume limits
  to 512. Old 558-byte resume files still load once, then save in the larger format.
- The Favorites queue is an explicit track list in `PlayQueue` and is saved in the resume file,
  so next/previous and restart stay inside Favorites.
- Sort keys keep leading articles: "The End of the World" sorts under T. An existing
  `library.db` needs a rescan to pick up the new order.
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
- Multiple Favorites lists are not implemented; the current Favorites root is one list of up to
  512 songs. See the future-ideas section in [README.md](README.md).
