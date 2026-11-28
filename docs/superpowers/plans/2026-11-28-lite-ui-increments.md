# Lite UI Increment Implementation Plan

Status: **M1–M4 code and incremental images built; final device check pending.** The rescan
performance item remains separate and is not part of this pass.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the approved Lite navigation, list, motion, orientation and Menu-key changes in four flashable increments.

**Architecture:** Keep the existing browse stack for Home and lists, add a raise-or-push operation for singleton overlays, use the existing strip renderer and canvas shift for transitions, and keep list selection immediate. Build one image after each milestone so a regression can be tied to one group of changes.

**Tech Stack:** C11, CMake, host tests, Lite panel tests, release-lite image, `tools/make_image.py`.

**Specs:**
- `docs/superpowers/specs/2026-11-28-lite-navigation-design.md`
- `docs/superpowers/specs/2026-11-28-list-section-jump.md`
- `docs/superpowers/specs/2026-11-28-lite-list-headers.md`
- `docs/superpowers/specs/2026-11-28-lite-motion-system.md`
- `docs/superpowers/specs/2026-11-28-update-icon-loss.md`

## Global Constraints

- 512 MB Echo Mini only; the 8 GB model is not supported.
- Do not modify the boot chain, IDB, PMU firmware, fw2 or slot layout.
- Runtime Light/Dark and Landscape/Portrait/Portrait-flipped must work.
- Preserve the Lite visual identity: dark card borders `#52525B`, shadows `#000000`, no white
  borders.
- Keep list selection immediate; do not animate every row move.
- No full-screen crossfade; use the strip renderer, horizontal page slides and a short overlay
  sheet.
- Keep `code window`, `loaded total`, `resident tables`, `rescan overlay` within their limits.
- Do not add resident tables; the resident-table limit has 184 bytes free.
- Run host tests with `ctest --preset dev -E "^format$"` and Lite panel tests with
  `test_lite_panel`.
- Commit each milestone separately and build one image per milestone.

## Review Focus

- A double Menu must not make a single Menu action fire as a side effect.
- Back from Settings must reveal the screen that opened it, not a different copy.
- Repeated Menu/Menu/Back/panel cycles must not grow the stack.
- Fullwidth/compatible Latin must not create a non-monotonic `D, #, D` section list.
- The volume hold must never change meaning mid-gesture.
- All Songs and Up Next landscape must clear the status meter without moving it.
- Portrait-flipped must rotate the panel, boot image and rocker consistently.
- The animation setting must scale every UI transition but not the marquee or faces.

---

## M1: Navigation and Menu key

### Task 1: Add the double-press behavior without repeat

**Files:**
- Modify: `src/core/include/core/key_tracker.h`
- Modify: `src/core/key_tracker.c`
- Test: `tests/core/test_key_tracker.c`

**Interfaces:**
- Produces: `KEY_BEHAVIOR_TAP_HOLD_DOUBLE`, a tap/long/double behavior with no repeat.
- `key_tracker_key_down`, `key_tracker_key_up` and `key_tracker_update` keep their signatures.

- [ ] Add a failing test: a short tap emits only after the 200 ms window; a second press inside
      the window emits one `DOUBLE_PRESS`; a long hold emits `LONG_PRESS` and no repeat.
- [ ] Add the enum value and share the double-pending logic with the existing
      `TAP_HOLD_REPEAT_DOUBLE`; do not make the new behavior repeat.
- [ ] Run the key-tracker test; then the full host suite.

### Task 2: Add singleton overlay operations to the stack

**Files:**
- Modify: `src/ui/include/ui/screen.h`
- Modify: `src/ui/include/ui/screen_stack.h`
- Modify: `src/ui/screen_stack.c`
- Test: `tests/ui/test_screen_stack.c`

**Interfaces:**
- `bool screen_stack_show(ScreenStack *stack, Screen *target, uint32_t now_ms)` raises an
  existing screen (popping everything above it) or pushes it once; returns false when it is
  already the top.
- `Screen` gains `bool overlay;`, `bool double_menu_enabled;` and
  `ScreenAction (*double_menu)(void *context, uint32_t now_ms);`.

- [ ] Add tests for push, raise to an existing screen, repeated raise (no growth), and pop-to-
      existing from several overlays.
- [ ] Implement `screen_stack_show`; keep `screen_stack_apply` for ordinary browse pushes.
- [ ] Run the UI tests.

### Task 3: Apply the B navigation model

**Files:**
- Modify: `src/app/app.c`
- Modify: `src/app/screens/now_playing_screen.c`
- Modify: `src/app/screens/now_playing_screen.h`
- Modify: `src/app/screens/playback_panel_screen.c`
- Modify: `src/app/screens/up_next_screen.c`
- Modify: `src/app/app_screens.c`
- Test: `tests/app/test_lite_panel.c`
- Test: `tests/app/test_app.c`

**Interfaces:**
- Mark Now Playing, the playback panel, Settings, Up Next and the A-Z overlay as `overlay`.
- `app` uses `screen_stack_show` for global Menu/Long Menu overlay jumps; browse pushes stay on
  `screen_stack_apply`.
- `Now Playing` short Menu opens Up Next (or skips only in non-Lite); long Play is a no-op.
- Settings Back pops to its opener; Settings short Menu opens Now Playing and closes Settings
  and any panel under it; long Menu and long Play are no-ops.
- Lite ignores `OPTION_MENU_KEY` and `menu_is_media`; the Menu key always means Up Next.
- The Settings Menu-key row is hidden only in Lite; the shared option slot remains.

- [ ] Add failing panel tests for: Settings over panel Back -> panel; Settings Menu -> Now
      Playing and panel gone; panel Menu -> Now Playing; long Play in Now Playing/Settings is a
      no-op; repeated Menu cycles do not grow the stack.
- [ ] Set the overlay flags and callbacks, replace the ad-hoc `now_playing_action()` and
      `settings_open()` policy, and add `open_overlay` in `app.c`.
- [ ] Hide the Lite Menu-key row with `choice_shown`; force `menu_skips = false` and
      `menu_is_media = false` in Lite.
- [ ] Run the panel and app tests.

### Task 4: Add the double-Menu callbacks

**Files:**
- Modify: `src/app/screens/now_playing_screen.c`
- Modify: `src/app/screens/now_playing_screen.h`
- Modify: `src/app/settings_screens.c`
- Modify: `src/app/settings_screens.h`
- Modify: `src/app/settings_scene.c`
- Modify: `src/app/app.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- Now Playing: `double_menu` returns a push of the playback panel.
- Settings: `double_menu` calls `settings_next_category(SettingsScreens *, uint32_t now_ms)`
  and returns no stack action.
- The app sets `KEY_MENU` to `TAP_HOLD_DOUBLE` only when
  `top->double_menu_enabled` and the display is on; otherwise it uses `TAP_OR_HOLD`.

- [ ] Add tests that double Menu opens the panel from Now Playing, advances the Settings
      category, and does not fire short Menu.
- [ ] Implement the callbacks and behavior choice; keep Quick Panel double Menu unassigned.
- [ ] Run the tests.

### Task 5: Build M1

- [ ] Run host `ctest --preset dev -E "^format$"` and `test_lite_panel`.
- [ ] Build `release-lite`, check the updater, save the image as the next `MINIV` number, and
      record SHA-256. Do not flash from the host.

---

## M2: Lists and section jump

### Task 6: Make the section initial use the sort key

**Files:**
- Modify: `src/library/sort_initial.c`
- Test: `tests/library/test_sort_key.c`
- Test: `tests/app/test_lite_panel.c`

- [ ] Add failing cases for `D`, `Ｄ`, `Da`, `Ａbba`, accented Latin and CJK/kana.
- [ ] Make `sort_key_initial()` call `sort_key_make()` into a small buffer and read the first
      effective code point; keep the image pinyin fallback for CJK when the pinyin table is not
      loaded.
- [ ] Verify the fullwidth `Ｄ` section is the same as `D`; run the library tests.

### Task 7: Remove section jump from the volume hold

**Files:**
- Modify: `src/ui/include/ui/list_view.h`
- Modify: `src/ui/list_view.c`
- Test: `tests/ui/test_fast_scroll.c`

- [ ] Change the tests: a long hold accelerates rows only; it never selects a different
      section and never shows a section box.
- [ ] Remove the section-box state and drawing, keep the 1.5 s / 4-row and 3 s / 12-row
      acceleration, and remove the no-section special case.
- [ ] Run the UI tests.

### Task 8: Add the A-Z/# overlay

**Files:**
- Create: `src/app/screens/section_jump_screen.c`
- Create: `src/app/screens/section_jump_screen.h`
- Modify: `src/app/CMakeLists.txt`
- Modify: `src/app/browse_screen.c`
- Modify: `src/app/browse_screen.h`
- Modify: `src/ui/list_view.c`
- Modify: `src/ui/include/ui/list_view.h`
- Test: `tests/app/test_lite_panel.c`
- Test: `tests/ui/test_fast_scroll.c`

**Interfaces:**
- `list_view_jump_to_section(ListView *, int section, uint32_t now_ms)`
- `bool list_view_sections_vary(const ListView *)`
- `int list_view_section_of(const ListView *, int index)`
- `SectionJumpScreen` draws a 7 x 4 `# A-Z #` grid, selects with volume, jumps with Play, and
  closes with Back/Menu.
- Each `BrowseScreen` has a `double_menu` callback that opens the overlay when sections vary.

- [ ] Add failing tests for the section jump target, the overlay opening only on double Menu,
      Play jumping to the first row of the letter, and Back returning to the list.
- [ ] Implement the overlay and browse integration; mark the overlay screen `overlay` and
      `double_menu_enabled = false`.
- [ ] Run the panel and UI tests.

### Task 9: Remove the All Songs title and inset Up Next

**Files:**
- Modify: `src/ui/lite_frame.c`
- Modify: `src/ui/include/ui/lite_frame.h`
- Modify: `src/app/browse_screen.c`
- Modify: `src/app/screens/up_next_screen.c`
- Test: `tests/app/test_lite_panel.c`

- [ ] Add `Rect lite_list_frame_no_title(Canvas *, const Theme *)` with the same card/rows
      layout, no title/rule.
- [ ] Use it for Lite All Songs in both orientations.
- [ ] Use it for Up Next landscape only; portrait keeps the current content area.
- [ ] Add pixel/layout tests proving no title text, first row below the meter, card at the
      expected y, and portrait unchanged.
- [ ] Build M2 and record SHA-256.

---

## M3: Motion and portrait flipped

### Task 10: Unify Home with the page motion

**Files:**
- Modify: `src/app/app_screens.c`
- Test: `tests/app/test_lite_panel.c`

- [ ] Replace the fixed 360 ms smoothstep with `settings_animation_ms(options, SLIDE_DURATION_MS)`
      and the same cubic ease-out used by `slide.c`.
- [ ] Add a test that Home uses the animation setting and reaches its target at the deadline.
- [ ] Keep the selected shadow behavior for now; it is not part of this pass.

### Task 11: Add the overlay sheet

**Files:**
- Modify: `src/gfx/include/gfx/canvas.h`
- Modify: `src/gfx/canvas.c`
- Modify: `src/app/app.c`
- Test: `tests/gfx/test_canvas.c`
- Test: `tests/app/test_lite_panel.c`

- [ ] Add `Canvas canvas_shifted_y(const Canvas *, int dy, Rect area)` and tests for clipping
      and reverse shift.
- [ ] Add `app.slide_vertical`; choose a 160 ms scaled overlay duration and a 20 px travel.
- [ ] Draw page transitions horizontally as today; draw overlay transitions as a sheet:
      forward draws the leaving screen at 0 and the arriving screen at 20 -> 0; back draws the
      arriving screen at 0 and the leaving screen at 0 -> 20.
- [ ] Keep the status meter on top and do not use a dimming scrim.
- [ ] Test forward/back offsets and that page transitions stay horizontal.

### Task 12: Animate Settings category changes

**Files:**
- Modify: `src/app/settings_screens.h`
- Modify: `src/app/settings_screens.c`
- Modify: `src/app/settings_scene.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- `settings_next_category(SettingsScreens *, uint32_t now_ms)` is public.
- `SettingsScreens` keeps the previous group, selected row, rows and a previous `MenuScreen`
  for one 140 ms transition.
- `settings_show_category()` uses the real `now_ms` for `list_view_select` and the marquee.

- [ ] Add a failing test for the real marquee time base and the category transition state.
- [ ] Copy the old rows/group/selection, swap to the new rows, and draw the old scene shifted
      left and the new scene shifted right for 140 ms; use the animation setting.
- [ ] Run the panel tests.

### Task 13: Add Portrait-flipped to Lite Settings

**Files:**
- Modify: `src/app/settings_screens.c`
- Modify: `src/app/settings_screens.h`
- Test: `tests/app/test_lite_panel.c`
- Test: `tests/settings/test_rocker.c`

- [ ] Change `LITE_ORIENTATION_OPTIONS` to three entries and the choice count to
      `ORIENTATION_COUNT`.
- [ ] Add a test that the third option is selectable, rotates the display, uses the flipped
      boot still and keeps the rocker mapping correct.
- [ ] Build M3 and record SHA-256.

---

## M4: Final pass

- [ ] Run the full host suite, Lite panel suite, generic device build, release-lite build and
      updater check.
- [ ] Build the final image; record SHA-256 and size headroom.
- [ ] Update `docs/lite/install-512m.md`, shortcuts/README wording, and the verification notes.
- [ ] Review the diff against all specs; no unimplemented approved rule may remain silently
      open. Leave the rescan-performance item separate.

---

## Next milestones

### M5: Settings absorbs the Quick Panel

- Add the Quick category with playback-mode and sleep-timer cards.
- Simplify to Quick / Sound / Display / System; fold Buttons and Library, move Lyrics to
  Display, remove the duplicate Sleep row.
- Remove the standalone Quick Panel and its overlay entry.
- Long Menu opens Settings/Quick from Home, Now Playing and non-lettered lists; A-Z stays in
  lettered lists; Settings Long Menu goes to the next category.
- Remove double Menu; use a 400 ms long-Menu threshold for Lite.
- Tests: quick cards, four categories, Settings Back, long-Menu mapping, no panel overlay.

### M6: Local motion

- Remove the overlay sheet; overlay panels appear and disappear directly.
- Settings category changes keep the rows static and animate only the badge, glyph and category
  dots.
- Keep page slides and the Home carousel.
- Tests: no overlay slide, dot interpolation, rows do not move, page slides unchanged.

### M7: Song favorites

- Add the path-hash song-favorites store and card file.
- Replace the Home Playlists card with Favorites.
- Add a favorite-song list and Add/Remove Favorites in the song actions menu.
- Decide and implement the queue: a real favorites queue is preferred; starting All Songs is
  only the fallback.
- Write a separate implementation plan before coding; this is larger than a Home-card rename.
- Album-favorites cleanup follows after M7, not as its own image.

### Separate

- Rescan performance/stutter.
- Any further Quick/Settings visual tuning after M5-M7 device checks.
