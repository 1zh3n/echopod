# Settings and the Quick Panel merge

Status: **direction approved: Settings absorbs the Quick Panel; details open, not implemented.**
Recorded 2026-11-28.

## Current structure

- **Settings:** singleton overlay, five category screens (Sound / Display / Library / Buttons /
  System), each with a card list and category dots.
- **Quick Panel:** a separate full-screen overlay with two large cards: playback mode and sleep
  timer. It is not a settings menu.
- Both are singleton overlays in the navigation model.

The question: upgrade the Quick Panel into a settings menu and let it replace the current
Settings.

## Direction

Settings absorbs the Quick Panel. The standalone Quick Panel is removed; its two cards become a
**Quick** category at the front of Settings.

Proposed simplified Settings:

| Category | Rows |
|---|---|
| Quick | playback mode card, sleep timer card |
| Sound | gain, output, filter, balance, Bluetooth level, equalizer |
| Display | theme, orientation, brightness, timeout, animations, lyrics |
| System | language, date/time, auto-off, Bluetooth, USB mode, USB prompt, music folders, rescan, swap volume, power on |

This removes the two-row Buttons category and the three-row Library category. Lyrics moves to
Display; music folders and rescan move to System; the Menu-key row stays hidden. The Sleep row is
removed from System because the Quick card owns it. Four category dots replace five.

The Quick category is still a `MenuScreen` list with two items; Volume selects a card and Play
changes its value. The scene draws those two rows as the existing large cards. No separate
key handler is needed.

Key entry proposal:

- Short Menu: Now Playing / Up Next, immediately.
- Long Menu: Settings Quick from Home, Now Playing and non-lettered lists; A-Z in lettered
  lists; next category in Settings; no action in the A-Z overlay.
- No double Menu. Lite long Menu threshold 400 ms.
- Settings opens at Quick; Back returns the opener. The Home Settings card remains.

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

**C is chosen:** Settings gains a Quick category, the standalone Quick Panel disappears, and
Long Menu opens Settings/Quick from Now Playing. This preserves full Settings and keeps playback
mode/sleep one step closer. A is only the fallback if the Settings redesign has to be postponed;
D is rejected.

## Effects of C

- Settings category count becomes four (Quick / Sound / Display / System); dots and group row
  tables change. The old Buttons and Library groups are folded into System/Display.
- Playback mode and sleep move from the panel into the Quick category; their card drawing moves
  with them.
- Remove the panel from the overlay set and from the navigation key table.
- Long Menu in Now Playing becomes Settings/Quick instead of a separate panel.
- Tests: category transitions, quick card values, Long Menu/Back from Now Playing, and no
  duplicate Settings overlay.
- The code and RAM cost is modest; the UI design and edge cases are the work.

## Splitting Settings onto Home

Assessed 2026-11-28.

"Put Settings on Home" can mean two different things:

1. **Put the four Settings categories on Home as cards.** Home would become All Songs,
   Favorites, Quick, Sound, Display, System. The user would still have to open a category to see
   its rows, so this adds a Home step rather than removing one. Volume-card cycling over six
   cards is slower than Long Menu -> Settings.
2. **Put every Settings row on Home.** That is about 24 rows, including folders, Bluetooth,
   rescan, equalizer and language. It destroys the category grouping, turns Home into a settings
   dashboard and makes the music cards compete with settings for the same carousel.

Neither fits the current one-row Home carousel. Settings has subpages and stateful values that
do not fit a card. The only sensible Home-level settings are the two Quick cards, and those
already have a better home as the first Settings category.

Recommendation: keep Settings as one overlay with Quick first; keep Home as the two-card music
launcher. Do not split the Settings tree onto Home. If a visible entry is wanted, keep a third
Settings card, but do not turn its categories or rows into Home cards.

## Still open

- C or A.
- If C: Quick as first category or a tab; how to show two 140x112 cards inside the Settings card
  area in landscape and portrait.
- If C: whether the Home Settings card stays as a second entry.
