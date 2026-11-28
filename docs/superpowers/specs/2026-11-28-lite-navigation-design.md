# Lite navigation interaction design

Status: **implemented in MINIV935; final check in MINIV938.** Recorded 2026-11-28.

## Why this exists

The current screen stack is a raw history stack:

- every push appends, including a second copy of Now Playing;
- the stack holds 8 screens, then push silently fails;
- `settings_open()` checks the whole stack, so a long Menu behaves differently once Settings is
  buried;
- playback panel Menu has a special case that can turn into Back.

The observed result is a loop that feels wrong:

```text
Home              depth 1
All Songs         depth 2
Now Playing       depth 3
Settings          depth 4   (long Menu)
Now Playing       depth 5   (Menu in Settings pushes a second copy)
Back -> Settings  depth 4
Back -> Now Playing depth 3 (the older copy, not All Songs)
```

## User mental model

The user should only need three rules:

1. **Back** closes the current overlay; if there is no overlay, it goes up one browse level.
2. **Menu** goes to Now Playing; on Now Playing it opens Up Next (or skips, depending on the
   Menu-key setting).
3. **Long Menu** goes to Settings; in Settings it does nothing.

Long Play keeps its existing screen-specific meaning.

## Structure

- **Base:** Home and the browse stack (Artists, Albums, All Songs, folders, playlists, etc.).
- **Overlays (single instance):** Now Playing, playback panel (quick panel), Settings.
- **Transient overlay:** Up Next.

An overlay is not a history layer. It is brought to the front if it is already open, never
duplicated. Closing an overlay returns to what was underneath it.

## Key table

| Screen | Menu | Long Menu | Double Menu | Long Play | Back | Long Back |
|---|---|---|---|---|---|---|
| Home | Now Playing | Settings | no action | no action | up one browse level | Home |
| Lettered list | Now Playing | Settings | A-Z/# jump overlay | existing list action | up one browse level | Home |
| Now Playing | Up Next | Settings | Quick Panel | no action | list | Home |
| Playback panel | Now Playing | Settings | no action | no action | Now Playing | Home |
| Settings | Now Playing, closing the overlay | no action | next Settings category | no action | the screen that opened Settings | Home |
| Up Next | Now Playing | Settings | no action | no action | Now Playing | Home |

- Settings opened from Now Playing: Back -> Now Playing.
- Settings opened from the playback panel: Back -> playback panel.
- Settings opened from a list: Back -> that list.
- Menu from Settings always goes to Now Playing and closes Settings (and the panel if Settings was
  opened over it).
- Long Back goes Home from every overlay and list.
- Long Play in Now Playing, the playback panel and Settings stays a no-op for this pass.
- Long Menu in Settings stays a no-op for this pass.
- Double Menu is the group/navigation key: A-Z jump in lettered lists, next category in Settings,
  Quick Panel in Now Playing. It waits for the double-press window only where it is enabled.
- Lite always uses Menu for Up Next. The Menu-key setting should be hidden there (keep the shared
  settings slot for non-Lite); double-press volume already skips tracks.

## Sequences

The case that motivated this design:

```text
Now Playing
  --long Play--> panel
  --Menu--> Now Playing
  --long Menu--> Settings
  --Menu--> Now Playing (Settings closes; no second Now Playing)
  --Back--> list
```

From Settings over the panel:

```text
panel --long Menu--> Settings
Settings --Back--> panel
Settings --Menu--> Now Playing (Settings and panel close)
Now Playing --Back--> list
```

No repeated overlay, no stack depth growth, no silent push failure.

## Implementation notes for the later change

- Keep the browse stack for Home/lists.
- Add a singleton bring-to-front/pop-to-existing operation for Now Playing, playback panel and
  Settings. Use it for the global jumps instead of the current special cases.
- Remove the whole-stack `settings_open()` suppression as the key policy; Settings is a single
  overlay, not a flag that changes other screens' Menu.
- Keep Up Next as a transient overlay above Now Playing.
- If there is no current track, Menu does not open Now Playing (existing behavior); the overlay
  rules do not invent a fake track.
- Tests to add:
  - Settings -> Menu -> Now Playing leaves depth unchanged and Back goes to the list.
  - Panel -> Settings -> Back returns to the panel.
  - Panel -> Settings -> Menu returns to Now Playing and closes the panel.
  - Long Menu in Settings and long Play in the panel do nothing.
  - Repeated Menu/Menu/Back/panel cycles never grow the stack.

## Long Menu proposal (recorded, not implemented)

`MINIV938` uses double Menu for the group actions. The next proposal removes the double window
and moves the same actions to long Menu:

| Screen | Short Menu | Long Menu |
|---|---|---|
| Home | Now Playing | Settings / Quick |
| Lettered list | Now Playing | A-Z/# overlay |
| Other lists | Now Playing | Settings / Quick |
| Now Playing | Up Next | Settings / Quick |
| Settings | Now Playing | next category |
| Up Next | Now Playing | Settings / Quick |
| A-Z/# overlay | close | close |

- No double Menu. Short Menu acts immediately, so the 200 ms wait disappears.
- The standalone Quick Panel is removed: Settings absorbs its two cards as the Quick category.
  Long Menu opens Settings at Quick from Home, Now Playing and non-lettered lists.
- Lettered lists keep Long Menu for A-Z; Settings remains reachable from Home.
- Settings Back returns the opener. The Home Settings card stays.
- Lite lowers the Menu long-press threshold from 600 ms to 400 ms. Volume long press stays at
  1000 ms.
- Long Play remains the track context action; it is a no-op in Now Playing, Settings and Up Next.

Assessed risks: the group actions are slower than double tap, Settings from a lettered list
needs Long Back -> Home -> Settings, and long press is less discoverable. The proposal is
recorded, not implemented.

## Home decision

**Final direction 2026-11-28: keep the three-card Home.** The two-card Home and the no-Home
model were both rejected. The Home carousel is the Lite visual identity; a two-card version
loses the composition and removing it loses the screen entirely. The redundancy of a visible
Settings card is accepted because it is the price of keeping the design.

Final Home:

| Card | Behavior |
|---|---|
| All Songs | open All Songs |
| Favorites | open song Favorites (M7); replaces Playlists |
| Settings | open Settings at Quick; Long Menu is the shortcut |

- Keep the three-card carousel and the existing animation.
- Short Menu on Home: Now Playing.
- Long Menu on Home: Settings / Quick.
- Settings card is both a visual anchor and a discoverable fallback; the Long Menu shortcut
  remains.
- Playlists is removed from the Lite Home; the non-Lite menu keeps it.
- Do not add a fourth card or turn Home into a grid.

A separate productivity option remains a Lite Start screen setting (Home / All Songs /
Favorites / Now Playing), for people who want to skip Home at boot. It is not part of M5-M7.

### Rejected: two-card Home

A static two-card layout was considered and rejected: with only All Songs and Favorites it
still reads as a menu with no purpose, and it loses the three-card composition. Do not revive it
unless the Home concept is redesigned.

### Rejected: removing Home entirely

Removing Home is possible only if there is a replacement root switch between All Songs and
Favorites. The candidates are:

- **Short Back at the root**: toggles All Songs <-> Favorites. Deeper Back still goes up.
  Settings would need another gesture, for example Long Back; Long Back would no longer mean
  Home because there is no Home.
- **Long Back**: toggles the two roots from anywhere; then Settings needs a third gesture.
- A tab/filter inside one list: needs a new key or a selectable header row.

The two-card Home is already the direct switch: Volume selects, Play opens. That is one volume
press when switching from the other root. However, after the device discussion the two-card Home
was rejected as still feeling like a menu with no purpose. The no-Home model is now the
candidate, with this key map:

- root is All Songs or Favorites;
- Short Back at the root toggles All Songs <-> Favorites; deeper Short Back goes up;
- Long Back opens Settings;
- Short Menu opens Now Playing;
- Long Menu is A-Z in lettered lists, Settings/Quick in Now Playing and non-lettered lists, next
  category in Settings.

Trade-offs:

- Back at the root is no longer inert; it changes lists.
- Settings moves to Long Back; Long Back no longer means Home because there is no Home.
- Switching roots from a deep list needs repeated Short Back to the root.
- The Home visual identity and its visible Settings card disappear.
- Boot must choose All Songs or Favorites, or remember the last root.

Recommendation: **do not remove Home.** It was rejected because the visual identity matters more
than the small redundancy. A Start screen option can still skip Home at boot without deleting
the screen.

### Rejected: visual continuity without Home

The large three-card carousel cannot survive Home's removal; that composition is Home. The rest
of the Lite language can stay: hard shadows, dark card borders, color badges, the status meter,
the Settings category badge and the Now Playing icons.

To keep a visual anchor at the root, draw a **compact two-icon switcher** in the top-right of
All Songs and Favorites:

- All Songs icon and Favorites icon, about 24x24, same color-icon style;
- the active list has a selection outline;
- the status meter stays at top-left;
- rows still start at the existing card y=28;
- Short Back at the root toggles the active icon/list.

This is not a Home screen: it is a small root chrome element. It preserves the two-color icon
identity and makes the Back-to-switch behavior visible. If it feels too much like a tab bar, a
single active icon is the fallback; the list change still happens on Back.

If the big Home composition is the part that matters visually, then Home cannot be removed;
the two options are mutually exclusive.

## Up Next stays a queue

Up Next is not replaced by All Songs or Favorites. It is the playback queue view:

- row 0 is the playing track;
- rows after it are manually added tracks and the remaining queue;
- for a book it becomes the chapter list;
- it is the visible destination of Play Next / Add to Up Next;
- selecting a row jumps within the current queue.

All Songs and Favorites are already Home entries. Replacing Up Next with either would duplicate
a browse list and remove the only view of what will play next, especially after shuffle or an
album queue. The only gain would be a shortcut to All Songs/Favorites, which Home already
provides.

Possible small clarification later: section labels such as Playing / Added / Next, or a title
that says Queue. That is cosmetic and not part of the current change.
