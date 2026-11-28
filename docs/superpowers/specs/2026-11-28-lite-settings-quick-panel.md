# Settings and the Quick Panel merge

Status: **assessment, not implemented.** Recorded 2026-11-28.

## Current structure

- **Settings:** singleton overlay, five category screens (Sound / Display / Library / Buttons /
  System), each with a card list and category dots.
- **Quick Panel:** a separate full-screen overlay with two large cards: playback mode and sleep
  timer. It is not a settings menu.
- Both are singleton overlays in the navigation model.

The question: upgrade the Quick Panel into a settings menu and let it replace the current
Settings.

## Options

### A. Keep both, add a Settings card to the Quick Panel

- Quick Panel stays a small hub: playback mode, sleep, Settings.
- Selecting Settings opens the existing Settings overlay.
- Small change, no Settings redesign.
- Does not actually replace Settings; it only gives another entry.

### B. Make the Quick Panel a hub, keep Settings separate

- Quick Panel lists shortcuts (Settings, playback mode, sleep, maybe theme/brightness).
- Still two overlays, but the panel is a launcher rather than a settings `screen`.
- More useful than A, still does not replace Settings.
- More code for a launcher that duplicates the Home Settings card.

### C. Merge them into Settings with a Quick category

- Settings becomes the only overlay.
- Add a sixth `Quick` category with the playback-mode and sleep cards (or two rows).
- Remove the separate Quick Panel screen.
- Long Menu in Now Playing opens Settings/Quick; the Home Settings card remains.
- Category dots become six; the current Settings card layout must fit two quick controls.
- This is the cleanest single-overlay model and keeps the full Settings pages.

### D. Quick Panel replaces Settings completely

- All five Settings categories and their subpages must live in the Quick Panel.
- The two-card Quick Panel cannot hold that; it would need a scrollable category list, category
  switching, and the existing choice/subpage handling.
- This is effectively current Settings with a different frame, plus the two quick cards. It is
  the largest change and has the highest regression risk for little UX gain.
- Not recommended.

## Recommendation

Do **C** if the goal is one settings overlay: Settings gains a Quick category, the standalone
Quick Panel disappears, and Long Menu can open it from Now Playing. This preserves full
Settings and keeps playback mode/sleep one step closer.

Do **A** if the goal is only a faster Settings entry: add a Settings card to the existing Quick
Panel and keep both screens.

Do **D** only if the full Settings UI will be redesigned anyway; do not do it as a shortcut.

## Effects of C

- Settings category count 5 -> 6; dots and group row tables change.
- Playback mode and sleep move from the panel into the Quick category; their card drawing and
  key handling need a second home.
- Remove the panel from the overlay set and from the navigation key table.
- Long Menu in Now Playing becomes Settings/Quick instead of a separate panel.
- Tests: category transitions, quick card values, Long Menu/Back from Now Playing, and no
  duplicate Settings overlay.
- The code and RAM cost is modest; the UI design and edge cases are the work.

## Still open

- C or A.
- If C: Quick as first category or a tab; how to show two 140x112 cards inside the Settings card
  area in landscape and portrait.
- If C: whether the Home Settings card stays as a second entry.
