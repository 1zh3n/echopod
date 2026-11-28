# Lite song favorites

Status: **proposed, not implemented.** Recorded 2026-11-28.

## Context

The Lite Home carousel currently has All Songs, Playlists and Settings. Playlists are rarely
useful on this unit; a song Favorites list would be more useful.

The existing Favorites feature is not that feature:

- `FavoritesStore` and `library/favorites` store **album keys**;
- the Favorites browse screen and long-Play toggle are compiled only under `#ifndef ECHO_LITE`;
- Lite does not load the favorites store and has no entry point.

The shared album-favorites code remains live in the non-Lite source. Do not delete it while
adding song favorites.

## Direction

Replace the Lite Home **Playlists** card with **Favorites**, and make Favorites a song list.

## Proposed design

### Storage

- New file `/.echo/favorite_tracks.bin`, 4-byte keys, up to 256 tracks, about 1 KB.
- The key is a hash of the stored track path, so it survives a rescan reorder.
- `library_track_key()` reads the path from the library and hashes it; no new RAM table.
- Add the file to `card_files` only for Lite. The existing album favorites file stays for
  non-Lite.

### Home

- Home card 1 becomes Favorites, using `MUSIC_FAVORITES`.
- Playlists is removed from the Lite Home; the non-Lite menu keeps it.
- The Favorites card needs a heart/star-style icon. The current List icon does not read as
  favorites, so this is an asset/design task, not only a label change.

### Favorites list

- A track list, not an album list: title plus artist, sorted like All Songs.
- Empty state: "No favorites".
- Selecting a track starts the favorites list; next/previous should stay inside Favorites.
- A favorite indicator on rows is optional for the first pass.

### Adding and removing

Long Play on a track in All Songs, playlists, folders, albums or Up Next should open the song
actions menu with a third item:

- `Add to Favorites` when absent;
- `Remove from Favorites` when present;
- toast on add/remove/full.

This keeps the existing Play Next / Add to Up Next actions.

## Playback queue decision

The preferred design is a real favorites queue: starting any row makes the next/previous moves
stay in the favorites list. That needs an explicit track-list source in the player queue (or a
new `player_play_tracks` API), rather than reusing All Songs.

The lightweight alternative -- start the selected track in All Songs and continue there -- is
not a favorites queue and is not recommended.

## Album-favorites cleanup

Do not remove the shared album-favorites code now. The non-Lite UI uses it, and the source is
shared.

After song favorites is stable:

- keep the album-favorites implementation for non-Lite;
- remove Lite-only dead branches/entries that are provably unused;
- only remove album favorites globally if the project stops shipping the non-Lite UI, which is
  a separate decision.

## Still open

- Favorites queue: proper explicit track list or lightweight fallback.
- Heart/star icon design for the Home card.
- Whether Playlists should remain reachable anywhere else in Lite.
- Whether rows show a favorite marker.
