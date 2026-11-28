# Controls

These are the Lite controls. The boot/recovery button combinations are not changed.

| Screen | Volume | Play | Menu | Back |
|---|---|---|---|---|
| Home | change card | short: open | short: Now Playing; long: Settings | short: back; long: Home |
| Lists | change row; hold: repeat, faster after 1.5 s | short: open; long: song actions on tracks | short: Now Playing; long: Settings (A-Z/# jump in lettered lists) | short: back; long: Home |
| Song actions | change row | short: choose | short: close | short: close |
| Favorites | change row | short: play the favorites queue | short: Now Playing; long: Settings | short: back; long: Home |
| Now Playing | short: volume; double: previous/next; long: seek | short: pause/play | short: Up Next; long: Settings | short: leave; long: Home |
| Up Next | change row | short: play the row | short: Now Playing; long: Settings | short: back; long: Home |
| Settings | change row | short: change value / open | short: Now Playing; long: next category | short: back; long: Home |
| A-Z / # jump | select letter | short: jump | short: close | short: close; long: Home |
| Choice | change option | short: confirm | short: Now Playing; long: Settings | short: cancel; long: Home |

Long Menu uses a 400 ms threshold. There is no double-Menu gesture: a short Menu acts
immediately, and a long Menu opens Settings from Home, Now Playing, Up Next and non-lettered
lists, or the A-Z/# jump from a lettered list. In Settings it advances to the next category.

The Menu-key setting is removed from Lite: Menu always opens Up Next in Now Playing. Volume
double-press still does previous/next.

## Song actions

Long Play on a track in All Songs, an album, a folder, a playlist or Up Next opens the same
actions menu:

- Play Next;
- Add to Up Next;
- Add to Favorites, or Remove from Favorites when the track is already saved.

Adding or removing shows the existing favorites toast. The list is capped at 256 songs and
survives a rescan because it stores a hash of the track path, not a library index.

Long Play on a row in the Favorites list removes that song directly.

## Settings Quick cards

The standalone Quick panel is gone. Settings opens on Quick, which has two icon-only cards:

- playback mode: Random → Loop → Loop One → Random;
- sleep timer: Off → 15 → 30 → 45 → 60 → 90 → 120 → Off.

The sleep timer shows its remaining time and can wait for the current track before shutting
down. Long Play is a no-op in Settings.

## Long volume

Long volume only scrolls rows. The old ~3 s section jump is gone; use long Menu in a lettered
list for the A-Z/# overlay.
