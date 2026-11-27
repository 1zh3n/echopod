# Lite list headers and the status meter

Status: **proposed, not implemented.** Recorded 2026-11-28.

## What Up Next is

Up Next is a list, not a separate kind of screen:

- row 0 is the playing track;
- rows after it are added tracks and the queue;
- it uses the shared `ListView`, `ListModel`, and list navigation;
- in Lite it currently draws the generic `header_draw` title and rule, then the list below.

All Songs is also a list, but uses `lite_list_frame`, which draws its own title and rule.

## The overlap

On Lite panel screens the global status meter is drawn at `(8, 4)`: 28×8 plus a 2 px shadow,
so it covers `x = 8..38`, `y = 4..14`.

- All Songs title is drawn at `(16, 8)`; the meter covers its first characters.
- All Songs underline is at `y = 25`; it is below the meter but is redundant with the title.
- Up Next uses `header_draw`, whose title is also at `(16, 8)` and is covered the same way.

## Decision for these two screens

- **All Songs:** remove the text title and underline. Use the card/list layout without a
  header. The card stays at `y = 28`; rows start at `y = 30`.
- **Up Next:** do not move the list down. Remove the title and rule and draw it as the same
  no-title Lite list card at `y = 28`. Moving it down would only reduce the visible rows and
  would still leave the title covered.
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
- In Lite `up_next_screen`, replace `header_draw` + `list_view_draw` with the no-title
  `lite_list_frame` + `list_view_draw_over`, rows at the same `y = 30`.
- Tests: no title pixels in the top-left, first row starts below the meter, and the list
  still starts at the same row as before.
