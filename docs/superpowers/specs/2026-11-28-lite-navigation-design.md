# Lite navigation interaction design

Status: **approved interaction model, not implemented.** Recorded 2026-11-28.

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
| Now Playing | Up Next, or next track when configured | Settings | Quick Panel | no action | list | Home |
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

## Deferred

- Long Play in the playback panel closing the panel (would be a toggle). Kept as a no-op for
  now to avoid adding another key rule.
- A visible back-stack indicator or history menu. Not part of this pass.
