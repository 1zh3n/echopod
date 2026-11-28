# Controls

These are the Lite controls. The boot/recovery button combinations are not changed.

| Screen | Volume | Play | Menu | Back |
|---|---|---|---|---|
| Root list (All Songs / Favorites) | change row; hold: repeat, faster after 1.5 s | short: open | short: Now Playing; long: A-Z in All Songs, Settings in Favorites | short: switch root; long: Settings |
| Deeper list | change row; hold: repeat, faster after 1.5 s | short: open; long: song actions on tracks | short: Now Playing; long: Settings (A-Z in lettered lists) | short: up one level; long: Settings |
| Song actions | change row | short: choose | short: close | short: close |
| Favorites | change row | short: play the favorites queue | short: Now Playing; long: Settings | short: back to All Songs at the root; long: Settings |
| Now Playing | short: volume; double: previous/next; long: seek | short: pause/play | short: Up Next; long: Settings | short: leave; long: Settings |
| Up Next | change row | short: play the row | short: Now Playing; long: Settings | short: back; long: Settings |
| Settings | change row | short: change value / open | short: Now Playing; long: next category | short: return to the root; long: return to the root |
| A-Z / # jump | select letter | short: jump | short: close | short: close; long: close |

The root is All Songs or Favorites. Short Back at the root switches between them; deeper Short
Back goes up one list level. There is no Home screen. Long Back opens Settings from a root, a
list or Now Playing. Settings Short Back returns to whichever root was current when it opened.

Long Menu uses a 400 ms threshold. There is no double-Menu gesture: a short Menu acts
immediately. In Settings, Long Menu advances to the next category.

The Menu-key setting is removed from Lite: Menu always opens Up Next in Now Playing. Volume
double-press still does previous/next.

The two 24×24 root icons and their selection outline are deferred. List headers, rows, cards and
the status meter keep their existing M7 appearance.

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

Settings opens on Quick, which has two icon-only cards:

- playback mode: Random → Loop → Loop One → Random;
- sleep timer: Off → 15 → 30 → 45 → 60 → 90 → 120 → Off.

The sleep timer shows its remaining time and can wait for the current track before shutting
down. Long Play is a no-op in Settings.

## Long volume

Long volume only scrolls rows. The old ~3 s section jump is gone; use long Menu in a lettered
list for the A-Z/# overlay.
