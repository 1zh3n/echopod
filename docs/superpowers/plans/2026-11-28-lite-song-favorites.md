# Lite Song Favorites Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the Lite Home Playlists card with a song Favorites list, add/remove favorites from the song actions menu, and play the favorites as a real queue.

**Architecture:** Keep the existing album favorites for non-Lite. Add a path-hash song-favorites store for Lite, a dedicated favorites track list, and an explicit track-list source in `PlayQueue`/`Player` so next/previous stay inside Favorites. Persist the explicit list in the resume state so a restart restores the queue.

**Tech Stack:** C11, existing library/playback app code, host tests, Lite panel tests, release-lite image.

**Specs:**
- `docs/superpowers/specs/2026-11-28-lite-song-favorites.md`
- `docs/superpowers/specs/2026-11-28-lite-settings-quick-panel.md` (Home/Quick placement)

## Global Constraints

- 512 MB Lite build only for this feature.
- Do not change album favorites used by non-Lite.
- Do not add a heap allocation or a daily RAM table.
- Favorites are track paths, not library indices; keys must survive a rescan.
- Next/previous from Favorites must stay in Favorites.
- Keep the Home carousel at three cards.
- No comments in C source; `no_comments` must pass.
- Keep code window, loaded total, resident tables and rescan overlay within limits.
- Host tests: `ctest --preset dev -E "^format$"`; Lite: `test_lite_panel`.

## Review Focus

- A rescan that changes track order must not lose favorites.
- A favorite that disappears from the library must not crash or become a stale row.
- Starting the last favorite must not wrap into All Songs.
- Resuming after a restart must restore the favorites queue, not All Songs.
- Adding a favorite while the Favorites list is open must refresh the list or leave a consistent
  selection.
- A folder/album/Up Next track must offer the same Add/Remove Favorites action as All Songs.

---

### Task 1: Add a stable track key

**Files:**
- Modify: `src/library/include/library/library.h`
- Modify: `src/library/library.c`
- Test: `tests/library/test_library.c`

**Interfaces:**
- `uint32_t library_track_key(const Library *library, uint32_t track_index);`
- The key is a hash of the track path string. It is 0 only when the track cannot be read.

- [ ] Add a failing test with two tracks and two library fixtures: the same path gets the same
      key after rebuilding/reordering the library; different paths differ.
- [ ] Implement `library_track_key` with the existing streaming `hash_string` helper.
- [ ] Run `test_library` and the full library suite.

### Task 2: Add the Lite song-favorites store

**Files:**
- Modify: `src/app/favorites_store.h`
- Modify: `src/app/favorites_store.c`
- Modify: `src/app/card_files.c`
- Modify: `src/app/app.c`
- Test: `tests/app/test_stores.c`
- Test: `tests/library/test_favorites.c`

**Interfaces:**
- `extern const char FAVORITE_TRACKS_PATH[];`
- `void favorites_store_load_tracks(FavoritesStore *store, const Library *library);`
- `bool favorites_store_contains_track(const FavoritesStore *store, uint16_t track);`
- `FavoriteResult favorites_store_toggle_track(FavoritesStore *store, uint16_t track);`
- The existing album functions remain for non-Lite.

- [ ] Add a failing round-trip test for add/remove/full and reload from
      `/.echo/favorite_tracks.bin`.
- [ ] Refactor the common encode/decode/read path; load the album file only for non-Lite and the
      track file only for Lite. Use `library_track_key`.
- [ ] Add `FAVORITE_TRACKS_PATH` to the card files only under `ECHO_LITE`.
- [ ] In Lite `start_playback()`, load the track store; in non-Lite keep album loading.
- [ ] Run `test_stores` and the Lite build compile.

### Task 3: Add an explicit track-list queue

**Files:**
- Modify: `src/playback/include/playback/play_queue.h`
- Modify: `src/playback/play_queue.c`
- Modify: `src/playback/include/playback/resume_state.h`
- Modify: `src/playback/resume_state.c`
- Modify: `src/app/player.h`
- Modify: `src/app/player.c`
- Test: `tests/playback/test_play_queue.c`
- Test: `tests/playback/test_resume_state.c`
- Test: `tests/app/test_player.c`

**Interfaces:**
- `PLAY_QUEUE_TRACK_LIST_MAX = 256`
- `PlayQueuePlan` gains `const uint16_t *tracks; uint16_t track_count;`
- `PlayQueue` gains `uint16_t explicit_tracks[PLAY_QUEUE_TRACK_LIST_MAX];`
- `play_queue_build()` copies a non-empty explicit list into `queue->explicit_tracks` and points
  `queue->plan.tracks` at that copy.
- `bool player_play_tracks(Player *player, const uint16_t *tracks, uint16_t count, uint16_t position);`
- `RESUME_STATE_SIZE` grows to include the explicit count and 256 track indices.

- [ ] Add failing queue tests: first/next/previous/end stay inside an explicit 3-track list;
      the anchor starts at the requested position; a list with one track has no next.
- [ ] Add a failing resume test that saves a 3-track explicit queue and rebuilds the same
      current track after decode.
- [ ] Implement the explicit source in `PlayQueue` without changing normal BrowseQuery queues.
      Keep the source fields valid for resume validation even though the source is ignored.
- [ ] Add `player_play_tracks()` using the current shuffle/repeat rules and start it through
      `start_playable()`.
- [ ] Populate/restore the explicit list in `ResumeState` and `player_save`/`player_restore`.
      Existing 44-byte resume files become invalid; that is accepted and must not crash.
- [ ] Run the playback and player tests.

### Task 4: Add the Favorites track screen

**Files:**
- Create: `src/app/screens/favorite_tracks_screen.c`
- Create: `src/app/screens/favorite_tracks_screen.h`
- Modify: `src/app/CMakeLists.txt`
- Modify: `src/app/app_screens.h`
- Modify: `src/app/app_screens.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- `FavoriteTracksConfig` holds library, favorites store, player, now-playing screen and theme.
- `FavoriteTracksScreen` holds a `ListView`, `ListModel`, up to 256 track indices, row text
  caches and the favorites revision.
- Enter/update rebuilds the list when the revision changes; the list is in
  `LIBRARY_TITLE_ORDER` order.
- Selecting a row calls `player_play_tracks()` and pushes Now Playing.
- Long Play on a row removes it from Favorites and shows the existing favorite toast.

- [ ] Add failing Lite tests for an empty list, a two-track list, a row label, and starting the
      second favorite.
- [ ] Implement the screen using the same row cache style as Up Next.
- [ ] Add a no-title or titled Lite frame consistently with the existing list decisions.
- [ ] Run the Lite panel tests.

### Task 5: Replace the Home card

**Files:**
- Modify: `src/app/app_screens.c`
- Modify: `src/app/app_screens.h`
- Modify: `src/ui/lite_color_icons.c`
- Modify: `src/ui/include/ui/lite_color_icons.h`
- Modify: `assets/lite_icons.bin` / `assets/lite_icons.md`
- Test: `tests/app/test_lite_panel.c`
- Test: `tests/lite/test_lite_icon_asset.py` or the existing asset check.

**Interfaces:**
- Home card 1 is Favorites and opens `FavoriteTracksScreen`.
- Playlists is no longer in the Lite Home.
- `LITE_COLOR_ICON_FAVORITE` is a new 64x64 heart/star mask; do not reuse the List icon.

- [ ] Add failing Home tests: the middle card opens Favorites, the Playlists card is absent,
      and the icon asset generator reports no change.
- [ ] Create the favorite icon mask and regenerate `assets/lite_icons.bin`; update the icon
      offsets and `LITE_ICON_POINTER_LIMIT` if the face size changes.
- [ ] Wire the Home item directly to the Favorites screen; leave the non-Lite menu unchanged.
- [ ] Run the Lite panel and icon asset tests.

### Task 6: Add Favorites to the song actions menu

**Files:**
- Modify: `src/app/browse_screen.h`
- Modify: `src/app/browse_screen.c`
- Modify: `src/core/include/core/strings_menu.def`
- Modify: `assets/lang/*.txt` if the string pack requires the new labels
- Test: `tests/app/test_browse_screen.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- `SONG_ACTION_COUNT` becomes 3.
- `SongActions.items[3]` includes the dynamic Add/Remove Favorites item.
- The action uses `favorites_store_contains_track`/`toggle_track` and the existing favorite
  toast messages.

- [ ] Add a failing test for the three-item menu and the add -> remove toggle.
- [ ] Add the runtime label selection, action callback and toast.
- [ ] Ensure the action works from All Songs, album tracks, folder tracks and playlist tracks.
- [ ] Run the browse and Lite panel tests.

### Task 7: Cleanup and build

- [ ] Remove Lite-only dead album-favorite branches only after the song path is green.
- [ ] Run host tests, Lite panel tests, generic device build, release-lite and updater check.
- [ ] Build the next `MINIV` image; record SHA-256 and headroom.
- [ ] Update `docs/lite/shortcuts.md`, `docs/lite/customizations.md` and the verification notes.
- [ ] Leave rescan performance separate.

## Open questions before coding

- The Home Favorites icon: heart or star.
- Favorites list title: no title (like All Songs) or a titled list.
- Long Play in the Favorites list: remove directly, or open a small actions menu.
