# Lite controls, Settings cards and track lists

Status: **approved in conversation, implementation not started.** Recorded 2026-11-28.

## Intent

Make Lite's controls predictable: Short Menu is navigation, Long Menu is the screen's
function key, and Long Back is the one global Settings entry. Make Favorites behave like a
regular song list and make Settings navigable with cards instead of a hidden category gesture.
Simplify Now Playing's bottom row.

## Key model

| Screen | Short Menu | Long Menu | Short Back | Long Back |
|---|---|---|---|---|
| All Songs | Now Playing | A-Z/# jump | switch to Favorites at the root | Settings |
| Favorites | Now Playing | A-Z/# jump | switch to All Songs at the root | Settings |
| A-Z/# overlay | Now Playing | move to the next grid row | close | Settings |
| Now Playing | Up Next | current song actions | leave | Settings |
| Up Next | Now Playing | no action | back | Settings |
| Settings home | Now Playing | no action | current root | current root |
| Settings category | Now Playing | no action | Settings home | current root |
| Settings subpage | Now Playing | no action | previous Settings page | current root |

- Short Menu opens Now Playing whenever there is a current track and the top screen is not Now
  Playing. On Now Playing it opens Up Next. This also applies on the A-Z overlay.
- Long Menu never opens Settings. It is the current screen's function:
  - All Songs/Favorites: A-Z.
  - A-Z: next row (7 cells landscape, 4 portrait, wrapping).
  - Now Playing: song actions for the current track.
  - Up Next and Settings: no action.
- Long Back opens Settings from every screen except Settings descendants. In Settings home,
  category pages and subpages it returns to the current root.
- Short Back at the root still switches All Songs/Favorites. Short Back in a Settings category
  returns to Settings home; in a Settings subpage it returns to the category page.
- No double Menu. The Menu long-press threshold stays 400 ms.

## Track list architecture

Favorites should look and behave like All Songs, but it must not be forced into `BrowseScreen`.
`BrowseScreen` is built around `BrowseQuery`, library sections and resumable queue sources;
Favorites is an explicit track list backed by path-hash keys.

Introduce `TrackListScreen` as a reusable Lite screen for explicit track collections:

- owns a `ListView`, row cache, `SectionJumpScreen`, empty state and a 256-track buffer;
- collects through a small `TrackListSource` callback interface;
- caches by source revision and `library_fingerprint`; a root switch must not rescan the library;
- stops collecting after every source key has been found;
- services audio every 16 scanned tracks while collecting;
- rows show title + duration, like All Songs;
- Enter/Play starts `player_play_tracks()` so next/previous stay inside the collection;
- Short Menu and Long Menu follow the global key model;
- Long Play is supplied by the source (Favorites removes the song and shows the toast).

Favorites becomes the first `TrackListSource`; future explicit lists (recently played, imported
lists, etc.) can reuse the screen without touching `BrowseScreen` or non-Lite.

The A-Z overlay remains `SectionJumpScreen`; each `TrackListScreen` owns one instance. The
section is the first character of the row title, using the existing sort-key helper.

## Settings card model

Settings stops being one category per screen with Long Menu cycling. It becomes a small
hierarchy:

```text
Settings
  Random mode
  Sleep timer
  Sound        >
  Display      >
  System       >
```

- `Random mode` and `Sleep timer` are direct items; they keep their current actions and live
  values. Quick is no longer a category page.
- `Sound`, `Display` and `System` open category pages with the existing rows.
- Settings home has five compact cards: about 24 px high landscape (2 px gaps) and 44 px high
  portrait. Each card has a small colour marker, label, value or chevron.
- Settings home has no category badge and no category dots.
- Category pages keep the half-badge, glyph and category colour; the category dots are removed.
  Landscape keeps no text title; portrait keeps the category title.
- Category pages no longer animate between categories because categories are entered from the
  home cards. The local badge/glyph transition code is removed.
- Non-Lite Settings is unchanged.

Settings screens and their subpages (Equalizer, Folders, Date/Time, Bluetooth) are marked as
Settings descendants. Long Back from any of them returns to the current root.

## Now Playing

- Remove the always-on current/total time row.
- Remove the progress-bar thumb.
- Keep the transient volume number while changing volume.
- Seek feedback is the bar fill only; seek is a low-frequency operation.
- The existing layout boxes stay in place; the empty time area is unused except for transient
  volume text. No spectrum or title layout change is required for this pass.

## Constraints

- 512 MB Lite only; non-Lite Home, Settings and start paths stay unchanged.
- Preserve card borders, shadows, status meter and the existing list frame geometry.
- No heap allocation and no new resident tables.
- Host `ctest --preset dev -E "^format$"` and Lite `test_lite_panel`.
- Keep code window, loaded total, resident tables and rescan overlay within limits.
- Commit incrementally and build one image for the completed set.

## Review focus

- A-Z short Menu must open Now Playing without closing the overlay differently from other lists.
- A-Z long Menu must move exactly one grid row and wrap.
- Long Menu must never open Settings on any screen.
- Long Back from a Settings subpage must not duplicate Settings.
- Favorites must reuse its collected list on root switches and still refresh after a favorite or
  library change.
- Favorites A-Z must jump to the correct letter and return the selection to Favorites.
- Settings category pages must return to Settings home, not the root.
- Removing the time row must not remove transient volume feedback.

## Rejected alternatives

- Folding Favorites into `BrowseScreen`/`BrowseQuery`: leaks explicit track sources into the
  library browse and resume formats, and risks non-Lite behavior.
- 2x2 category grid: adds a step to the two most-used Quick controls.
- Inline accordion categories: two-level focus on the four hardware keys is awkward and the
  System page does not fit with three collapsed headers.
- Long Menu as Settings: conflicts with Long Back as the single Settings entry and leaves no
  function key.
- Short Menu no-op in Now Playing with Up Next on Long Menu: makes Up Next slower and wastes the
  main navigation key.
