# Lite controls, Settings cards and track lists Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Unify Lite Menu/Back semantics, make Favorites a reusable All-Songs-style track list with A-Z, replace Settings category paging with a five-card home, and simplify Now Playing.

**Architecture:** Add a generic `TrackListScreen` for explicit track collections and use Favorites as its first source. Add a Settings home MenuScreen with five compact cards and three category pages. Move Settings access to Long Back only, make Long Menu the screen function key, and adjust Now Playing/Up Next callbacks. Keep `BrowseScreen` and non-Lite behavior untouched.

**Tech Stack:** C11, existing Lite panel tests, host tests, release-lite image.

**Spec:** `docs/superpowers/specs/2026-11-28-lite-ui-controls-settings-lists-design.md`

## Global Constraints

- 512 MB Lite only; non-Lite Home, Settings, start paths and list behavior stay unchanged.
- Preserve Lite card borders, shadows, status meter and list frame geometry.
- No heap allocation and no new resident tables.
- Short Menu follows the global Now Playing rule; Long Menu never opens Settings.
- Long Back is the only global Settings entry; settings descendants return to the current root.
- Favorites rows show title + duration and support A-Z, like All Songs.
- Now Playing removes time text and the progress thumb but keeps transient volume numbers.
- Host tests: `ctest --preset dev -E "^format$"`; Lite: `test_lite_panel`.

## Review Focus

- Short Menu from the A-Z overlay opens Now Playing without leaving a second overlay.
- Long Menu in A-Z moves exactly one grid row and wraps.
- Long Menu never opens Settings on any screen.
- Long Back from a Settings subpage returns to the root instead of opening Settings twice.
- A root switch after the first Favorites visit reuses the collected list; a favorite/library change refreshes it.
- Favorites A-Z jumps to the correct letter and returns to the Favorites selection.
- Settings category pages return to Settings home, not the root.
- Removing the time row does not remove transient volume feedback.

---

### Task 1: Add the reusable TrackListScreen

**Files:**
- Create: `src/app/screens/track_list_screen.h`
- Create: `src/app/screens/track_list_screen.c`
- Modify: `src/app/CMakeLists.txt`
- Modify: `src/app/app_screens.h`
- Modify: `src/app/app_screens.c`
- Delete: `src/app/screens/favorite_tracks_screen.h`
- Delete: `src/app/screens/favorite_tracks_screen.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- `TrackListSource` supplies `revision`, `collect`, and `hold`:
  - `uint32_t (*revision)(void *context);`
  - `uint16_t (*collect)(void *context, uint16_t *tracks, uint16_t capacity);`
  - `ScreenAction (*hold)(void *context, uint16_t track, uint32_t now_ms);`
- `TrackListConfig` carries library, player, now-playing target, theme, toast, source and
  `StringId empty_message`.
- `TrackListScreen` owns `ListModel`, `ListView`, `SectionJumpScreen`, `tracks[256]`, row caches,
  `count`, `revision`, `library_stamp`, `rows_first` and `rows_count`.
- Favorites is implemented as a source in `app_screens.c`; `AppScreens` holds one
  `TrackListScreen favorite_tracks`.
- `track_list_screen_init()` returns the screen; `TrackListScreen` is the public type used by
  Lite tests.

- [ ] Add failing Lite tests: Favorites rows show duration; Favorites supports a section callback;
      A-Z long Menu opens the jump grid; choosing a letter returns to Favorites; long Play removes
      the favorite; a second Enter reads no library pages; adding a favorite refreshes the list.
- [ ] Move the favorite collection into a `TrackListSource`, keeping the path-hash key scan,
      audio service every 16 tracks, early stop when all keys are found, and revision/library
      fingerprint caching.
- [ ] Copy the row-cache/list behavior from the old Favorites screen into `TrackListScreen`; use
      `format_duration` for the detail and `sort_key_initial` for sections.
- [ ] Build the A-Z overlay from the screen's own `SectionJumpScreen`; set `long_menu_enabled`
      only when the visible sections vary.
- [ ] Select a row through `player_play_tracks()` and push Now Playing.
- [ ] Replace the `FavoriteTracksScreen` fields/tests with `TrackListScreen`.
- [ ] Run `test_lite_panel` and the host suite.
- [ ] Commit.

### Task 2: Move Settings to a five-card home

**Files:**
- Modify: `src/app/settings_screens.h`
- Modify: `src/app/settings_screens.c`
- Modify: `src/app/settings_scene.c`
- Modify: `src/app/app_screens.h`
- Modify: `src/app/app_screens.c`
- Modify: `src/app/app.c`
- Modify: `src/ui/include/ui/screen.h`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- Lite `SettingsScreens` gains `MenuScreen home`, `MenuItem home_items[5]` and three category
  links; `settings_screens_init()` returns the home screen in Lite and the existing menu in
  non-Lite.
- Settings home items, in order: Random mode, Sleep timer, Sound, Display, System.
- Sound/Display/System push `&screens->menu.screen` after showing that category.
- Lite `SettingsGroup` contains Sound, Display, System; Quick is no longer a group.
- `Screen` gains a `settings` flag. It is set on Settings home, category pages and the
  Equalizer/Folders/Date-Time/Bluetooth screens.
- Lite `app.c` opens Settings through the home screen; Short Back on Settings home pops to root;
  Long Back on any `screen->settings` screen pops to root.

- [ ] Add failing Lite tests: Settings opens on the five-card home; Random/Sleep are direct items;
      Sound/Display/System open category pages; category Back returns home; home Back returns the
      root; Long Back from a category/subpage returns root; category dots are gone.
- [ ] Build the Settings home MenuScreen and its card scene: five compact cards, 24 px landscape
      with 2 px gaps, 44 px portrait; colour marker, label, value or chevron.
- [ ] Reuse the existing quick callbacks/values for Random and Sleep; add category push callbacks.
- [ ] Remove Quick from `SettingsGroup`, adjust the Lite row indices, remove category dots and the
      category-to-category transition/badge blending from `settings_scene.c`.
- [ ] Mark Settings descendants and wire the Short/Long Back rules in `app.c`.
- [ ] Keep the non-Lite Settings path byte-for-byte in behavior; guard all new Lite fields/code.
- [ ] Run `test_lite_panel` and the host suite.
- [ ] Commit.

### Task 3: Unify Menu and Long Back

**Files:**
- Modify: `src/app/app.c`
- Modify: `src/app/screens/section_jump_screen.c`
- Modify: `src/app/screens/now_playing_screen.h`
- Modify: `src/app/screens/now_playing_screen.c`
- Modify: `src/app/screens/up_next_screen.c`
- Modify: `src/app/app_screens.c`
- Test: `tests/app/test_lite_panel.c`
- Test: `tests/app/test_up_next_screen.c`

**Interfaces:**
- Short Menu: open Now Playing when not already there; on Now Playing open Up Next.
- Long Menu:
  - All Songs/Favorites: A-Z.
  - A-Z: `SectionJumpScreen` long-menu callback moves by `LANDSCAPE_COLUMNS` or
    `PORTRAIT_COLUMNS`, wrapping.
  - Now Playing: `NowPlayingConfig.song_actions` callback for the current track.
  - Up Next: no action.
  - Settings: no action.
- Long Menu has no fallback that opens Settings.
- Long Back opens Settings from every screen except `screen->settings`; the A-Z overlay is no
  longer exempt.

- [ ] Add failing Lite tests: A-Z Short Menu opens Now Playing; A-Z Long Menu moves one row;
      Long Menu on Now Playing opens the current song actions; Long Menu on Up Next does nothing;
      Long Back on A-Z opens Settings; Long Menu on Settings does nothing.
- [ ] Add `song_actions`/context to `NowPlayingConfig`, set `long_menu` on Now Playing, and pass
      the current track plus title.
- [ ] Add the A-Z long-menu row movement; keep Short Back closing it.
- [ ] Remove the Settings fallback from the Lite Long Menu branch; remove the Settings category
      long-menu cycle.
- [ ] Remove the A-Z exception from `opens_now_playing` so Short Menu reaches Now Playing.
- [ ] Run `test_lite_panel`, `test_up_next_screen`, and the host suite.
- [ ] Commit.

### Task 4: Simplify Now Playing

**Files:**
- Modify: `src/app/screens/now_playing_screen.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- The time row draws only while volume feedback is visible; no current/total time otherwise.
- The Lite progress bar has no 6x10 thumb.
- The layout boxes stay in place; no spectrum/title geometry change.

- [ ] Add failing Lite tests: the time area is empty during normal playback; the progress bar has
      no thumb; volume feedback still draws its number.
- [ ] Remove the thumb fill in the Lite progress branch.
- [ ] Keep only the volume branch in `draw_times`; remove time formatting and duration text.
- [ ] Run `test_lite_panel` and the host suite.
- [ ] Commit.

### Task 5: Docs, image and verification

**Files:**
- Modify: `docs/lite/shortcuts.md`
- Modify: `docs/lite/customizations.md`
- Modify: `docs/lite/README.md`
- Modify: `docs/lite/install-512m.md`

- [ ] Document the Short Menu/Long Menu/Long Back model, the A-Z behavior, Favorites A-Z and
      duration rows, Settings five-card home, and Now Playing removal.
- [ ] Run host tests, Lite panel tests, generic device build, `release-lite`, updater check.
- [ ] Build the next `MINIV` image and record SHA-256 and headroom.
- [ ] Update the image table and verification notes.
