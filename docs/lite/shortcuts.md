# Controls

These are the Lite controls. The boot/recovery button combinations are not changed.

| Screen | Volume | Play | Menu | Back |
|---|---|---|---|---|
| Root list (All Songs / Favorites) | change row; hold: repeat, faster after 1.5 s | short: open | short: Now Playing; long: A-Z | short: switch root; long: Settings |
| Deeper list | change row; hold: repeat, faster after 1.5 s | short: open; long: song actions on tracks | short: Now Playing; long: A-Z in lettered lists | short: up one level; long: Settings |
| Song actions | change row | short: choose | short: Now Playing | short: close |
| A-Z / # jump | select letter | short: jump | short: Now Playing; long: next grid row | short: close; long: Settings |
| Now Playing | short: volume; double: previous/next; long: seek | short: pause/play | short: Up Next; long: current song actions | short: leave; long: Settings |
| Up Next | change row | short: play the row | short: Now Playing; long: no action | short: back; long: Settings |
| Settings home | change card | short: change value / open | short: Now Playing; long: no action | short: current root; long: current root |
| Settings category | change row | short: change value / open | short: Now Playing; long: no action | short: Settings home; long: current root |
| Settings subpage | change row | short: change value / open | short: Now Playing; long: no action | short: previous Settings page; long: current root |

**Short Menu is navigation:** it opens Now Playing; on Now Playing it opens Up Next. It does this on
every screen where a track is loaded, including the A-Z overlay.

**Long Menu is the current screen's function key:**

- All Songs / Favorites: A-Z jump.
- A-Z overlay: move to the next grid row (7 cells landscape, 4 portrait, wrapping).
- Now Playing: actions for the current track.
- Up Next and Settings: no action.

**Long Back is the single Settings entry:** from a root, list, Now Playing, Up Next or A-Z it opens
Settings. Inside Settings (home, category or a subpage such as Equalizer/Folders/Date-Time/
Bluetooth) it returns to the current root.

Short Back at the root switches All Songs and Favorites. In a Settings category it returns to the
Settings home; in a Settings subpage it returns to the category page.

There is no double Menu. The Menu long-press threshold is 400 ms.

## Favorites

Favorites is a track list like All Songs:

- rows show title and duration;
- the list is sorted by title and has the same A-Z/# jump;
- selecting a row starts the Favorites queue, so next/previous stay inside it;
- Long Play removes the selected song;
- the list is cached between root switches until a favorite or the library changes.

## Settings

Settings opens on five cards:

```text
Random mode
Sleep timer
Sound        >
Display      >
System       >
```

The compact rows have a colour dot on the left and scroll four at a time; the next row peeks at
the bottom. Random and Sleep are direct items with their values right-aligned. Sound, Display and
System open category pages. Category dots in the category pages are gone; the half-badge, glyph
and colour identify the category. The sleep row shows a live countdown.

## Now Playing

Now Playing has no current/total time row and no progress thumb. The progress bar is a flat line;
seek is shown only by the bar length. Changing volume still shows the volume number temporarily.
The top-right status blocks show Bluetooth, playback, then the always-visible mode block:
random is purple with no glyph, loop is gray with no glyph, and loop one is gray with a `1`.
There is no separate mode icon on the page.
