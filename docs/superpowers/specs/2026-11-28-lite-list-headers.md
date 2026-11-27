# Lite list headers and the status meter

Status: **implemented in MINIV936; final check in MINIV938.** Recorded 2026-11-28.

## What Up Next is

Up Next is a list, not a separate kind of screen:

- row 0 is the playing track;
- rows after it are added tracks and the queue;
- it uses the shared `ListView`, `ListModel`, and list navigation;
- in Lite `header_draw` is a no-op, so Up Next has **no title**. Its list starts at
  `theme_content_area`, which is `y = 0` in Lite landscape because `header_height = 0`.

All Songs is also a list, but uses `lite_list_frame`, which draws its own title and rule.

## The overlap

On Lite panel screens the global status meter is drawn at `(8, 4)`: 28×8 plus a 2 px shadow,
so it covers `x = 8..38`, `y = 4..14`.

- All Songs title is drawn at `(16, 8)`; the meter covers its first characters.
- All Songs underline is at `y = 25`; it is below the meter but is redundant with the title.
- Up Next has no title. Its **first list row starts at `y = 0`** in Lite landscape, so the
  meter covers the first row's text, not a header. In portrait `header_height = 40`, so the
  first row already starts below the meter.

## Decision for these two screens

- **All Songs:** remove the text title and underline. Use the card/list layout without a
  header. The card stays at `y = 28`; rows start at `y = 30`.
- **Up Next:** it already has no title. Move only the **landscape list** down to the normal
  no-title Lite card at `y = 28`, rows at `y = 30`. This is not a title removal; it is the
  top inset that clears the meter. Portrait already starts at `y = 40` and needs no change.
- The status meter stays at `(8, 4)`; the list rows are below it.

## Still open

Other Lite lists with titles (Artists, Albums, Album Artists, Genres, Folders, Playlists)
use the same `lite_list_frame` title position and therefore overlap the meter too. They were
not part of this request. Options after the two screens above:

1. shift those titles to the right of the meter (about `x = 48`) or below it;
2. remove titles from all Lite lists;
3. leave them as they are if the title context is worth the overlap.

Recommendation: fix All Songs and Up Next first, then look at the remaining lists on the
device before choosing a general title rule.

## Implementation notes for later

- Add a no-title variant of `lite_list_frame` (or a title flag) that keeps the card and
  returns the same row rectangle.
- Use it in Lite `browse_screen` for All Songs only.
- In Lite `up_next_screen`, keep the existing non-Lite path; for Lite landscape, replace
  `header_draw` + `list_view_draw` with the no-title `lite_list_frame` +
  `list_view_draw_over`, rows at `y = 30`. Portrait keeps the current content area.
- Tests: no title pixels in the top-left, first row starts below the meter, and the list
  still starts at the same row as before.
