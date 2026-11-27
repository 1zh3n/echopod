# Controls

These are the Lite controls. The boot/recovery button combinations are not changed.

| Screen | Volume | Play | Menu | Back |
|---|---|---|---|---|
| Home | change card | short: open | short: Now Playing; long: Settings | short: back; long: Home |
| Lists | change row; hold: repeat, faster after 1.5 s | short: open; long: context action if the screen has one | short: Now Playing; double: A-Z/# jump; long: Settings | short: back; long: Home |
| Now Playing | short: volume; double: previous/next; long: seek | short: pause/play | short: Up Next; double: Quick Panel; long: Settings | short: leave; long: Home |
| Playback panel | select tile | short: change current tile value | short: close; long: Settings | short: close; long: Home |
| Settings | change row | short: change value / open | short: Now Playing; double: next category; long: no action | short: back; long: Home |
| A-Z / # jump | select letter | short: jump | short: close | short: close; long: Home |
| Choice | change option | short: confirm | short: Now Playing; long: Settings | short: cancel; long: Home |

The double-press window is 200 ms, measured from the first release to the second press. A single
short Menu in a screen where double Menu is enabled is emitted at the end of that window.

The Menu-key setting is removed from Lite: Menu always opens Up Next in Now Playing. Volume
double-press still does previous/next, so the old Menu=Next option would duplicate it.

The playback panel has two tiles:

- playback mode: Random → Loop → Loop One → Random;
- sleep timer: Off → 15 → 30 → 45 → 60 → 90 → 120 → Off.

Long Play on a list triggers a context action only on screens that define one. It never acts as
a generic row switch. On Now Playing, the playback panel and Settings it is a no-op.

Long volume only scrolls rows. The old ~3 s section jump is gone; use double Menu for the
A-Z/# overlay.
