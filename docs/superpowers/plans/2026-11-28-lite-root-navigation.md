# Lite Root Navigation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the Lite Home carousel and boot directly into All Songs, with Short Back at the root toggling between All Songs and Favorites and Long Back opening Settings.

**Architecture:** Keep the existing browse stack and screens. Add a root-replacement operation to `ScreenStack`, expose the two root screens from `AppScreens`, and intercept the root Back/Long Back keys in the Lite app dispatcher. Do not add the M8 root-tab chrome yet: the existing list frames, headers and rows stay visually unchanged.

**Tech Stack:** C11, existing Lite panel tests, host tests, release-lite image.

**Spec:** `docs/superpowers/specs/2026-11-28-lite-navigation-design.md`

## Global Constraints

- 512 MB Lite build only.
- Keep all existing list visuals unchanged: no root-tab icons, no header-height change, no row or
  card change. The root-tab chrome is deferred.
- Root is All Songs or Favorites. Short Back at the root toggles; Short Back deeper pops one level.
- Long Back opens Settings from the root, lists and Now Playing. Settings Short Back returns the
  current root.
- Short Menu opens Now Playing. Long Menu keeps its M5/M7 meanings: A-Z in lettered lists,
  Settings in Now Playing/non-lettered lists, next category in Settings.
- Boot to All Songs. The Start screen setting is not added to Lite.
- Preserve the status meter and card `y=28`.
- Do not change non-Lite Home behavior.
- Run host `ctest --preset dev -E "^format$"` and Lite `test_lite_panel`.

## Review Focus

- Short Back at a child level must pop the child, not switch roots.
- Short Back at the Favorites root must return to All Songs, and Settings Back must return to
  whichever root was current when Settings opened.
- Long Back must not be swallowed by a screen's normal Back handler.
- All Songs root must keep its A-Z long Menu; Favorites root must open Settings on long Menu.
- Starting a favorite from the Favorites root must still play the Favorites queue, not All Songs.
- Boot must not depend on a Home screen or a saved Start screen value.

---

### Task 1: Replace the stack root

**Files:**
- Modify: `src/ui/include/ui/screen_stack.h`
- Modify: `src/ui/screen_stack.c`
- Test: `tests/ui/test_screen_stack.c`

**Interfaces:**
- `bool screen_stack_replace_root(ScreenStack *stack, Screen *root, uint32_t now_ms);`
- Replaces `screens[0]`, drops any deeper entries, enters the new root and returns true only when
  the root actually changed. A NULL or same root returns false.

- [ ] Add a failing test for replacing the root, checking depth becomes 1, the new root is on
      top, its `enter` ran, and replacing with the same root returns false.
- [ ] Implement the function in `screen_stack.c`.
- [ ] Run `test_screen_stack`.
- [ ] Commit.

### Task 2: Boot into All Songs and expose both roots

**Files:**
- Modify: `src/app/app_screens.h`
- Modify: `src/app/app_screens.c`
- Modify: `src/app/app.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- `AppScreens` gains `Screen *root_all_songs; Screen *root_favorites;` under `ECHO_LITE`.
- `app_screens_init()` returns the All Songs screen (or `no_library` when the store is
  unavailable) in Lite, and keeps the Home screen in non-Lite.
- Lite `app_screens_start_path()` returns 0 for every Start screen.

- [ ] Add failing Lite tests: boot title is `All Songs`; `START_ALBUMS` still boots `All Songs`.
- [ ] Build the All Songs root after `build_music()`; store the Favorites screen pointer; return
      the All Songs root. Use `no_library` when `context.store->available` is false.
- [ ] Disable Start screen paths in Lite.
- [ ] Update `open_all_songs_and_play()` and the affected Lite helpers so root All Songs is the
      starting screen (remove the old Home-to-All-Songs tap).
- [ ] Run `test_lite_panel`.
- [ ] Commit.

### Task 3: Root Back and Long Back

**Files:**
- Modify: `src/app/app.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- Short Back at stack depth 1 replaces the root with the other root, without animation.
- Long Back opens Settings from any top screen except Settings; in Settings it keeps the
  existing pop-to-root behavior.
- Long Menu still opens A-Z in All Songs and Settings in Favorites.

- [ ] Add failing Lite tests: Short Back toggles All Songs <-> Favorites at the root; Short Back
      from Now Playing returns to the current root; Long Back opens Settings from the root.
- [ ] Add failing Lite tests: Settings opened from Favorites returns to Favorites on Short Back;
      All Songs still opens A-Z on Long Menu; Favorites opens Settings on Long Menu.
- [ ] Intercept Short Back at depth 1 and call `screen_stack_replace_root` with the other root.
- [ ] Extend `opens_settings()` to accept Long Back in Lite, keeping the Long Menu behavior.
- [ ] Run `test_lite_panel`.
- [ ] Commit.

### Task 4: Remove the Home carousel

**Files:**
- Modify: `src/app/app_screens.h`
- Modify: `src/app/app_screens.c`
- Test: `tests/app/test_lite_panel.c`

**Interfaces:**
- No Lite Home menu, scene, items, art, root Home tests or `app_screens_home_key`.
- Non-Lite Home stays unchanged.

- [ ] Delete the Lite Home build/scene/key code and guard the Home fields to non-Lite.
- [ ] Remove the Home-only Lite tests and replace their status-meter coverage with a root-list
      test.
- [ ] Run `test_lite_panel` and the host suite.
- [ ] Commit.

### Task 5: Docs, image and verification

**Files:**
- Modify: `docs/lite/shortcuts.md`
- Modify: `docs/lite/customizations.md`
- Modify: `docs/lite/README.md`
- Modify: `docs/lite/install-512m.md`

- [ ] Document the root model, deferred root tabs, and the Boot/All Songs choice.
- [ ] Run host tests, Lite panel tests, generic device build, `release-lite`, updater check.
- [ ] Build the next `MINIV` image and record SHA-256 and headroom.
- [ ] Update the image table and verification notes.
