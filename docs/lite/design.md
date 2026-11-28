# Lite design

The final Lite design is a neo-brutalist interface with runtime Light/Dark and landscape/portrait.
The mockup sheet is [mockups/index.html](mockups/index.html).

> The Home sheet is the pre-M8 design (Home was removed); Now Playing, Settings and Quick panel are design mockups, not device screenshots.
> The standalone Quick panel sheet is historical: Quick is split into direct items on the Settings home.
> The boot image is rendered from the actual firmware assets.

## Palette

| Token | Light | Dark |
|---|---|---|
| page | `#FAFAF9` | `#0B0B0D` |
| card | `#FFFFFF` | `#17171C` |
| raised / frame | `#FFFFFF` | `#1F1F26` |
| primary text | `#0A0A0A` | `#F4F4F5` |
| secondary text | `#4B5563` | `#A1A1AA` |
| tertiary text | `#6B7280` | `#71717A` |
| card border / title rule | `#0A0A0A`, 2 px | `#52525B`, 1 px |
| badge border | `#0A0A0A`, 2 px | `#F4F4F5`, 2 px |
| divider / track | `#D1D5DB` | `#3F3F46` |
| card and badge shadow | `#0A0A0A` | `#000000` |
| selection | `#006FEE` | `#006FEE` |

Badge colours: blue `#2FA8FF`, orange `#FF9F1C`, purple `#8B5CF6`, indigo `#6366F1`,
cyan `#35C7E8`, amber `#F59E0B`, success `#22C55E`.

## Spacing

- Base unit: 4 px. Outer gutter: 8 px left/right/bottom on panel screens
  (Settings, Lists, Now Playing).
- Landscape header band: 28 px. Status markers use the row from y=8 to y=18; the list card
  starts at y=28.
- Portrait headers stay editorial: Settings card starts at y=58, Lists at y=40.
  Their cards end 8 px above the bottom edge.
- Now Playing uses 8 px top/sides/bottom. Content width is 304 px landscape and
  154 px portrait; the progress bar ends 8 px above the bottom edge.
- Settings home is an intentional compact-card exception inside the normal content area.
- Root lists use the existing no-title list frame and card at `y=28`; the left root switch is
  the segmented 28×10 meter.

## Layout

### Home (removed in M8)

- The large three-card carousel is gone. All Songs and Favorites are roots; Short Back switches
  them and Long Back opens Settings.
- The two 24×24 root icons and the animated selection outline are deferred, so the list frames
  stay unchanged for now.

### Root status

- Status uses 10×10 blocks with a 1 px hard shadow at y=8.
- Right side: Bluetooth `#0F8ADB` and playing `#F59E0B`, 3 px apart, 8 px from the right edge;
  inactive blocks are muted versions of their colours.
- Root lists use the original 28×10 segmented meter on the left: 2 px border/shadow, two 12×6
  cells; All Songs `#2FA8FF`, Favorites `#EC4899`, with the inactive half divider gray.
- Now Playing uses the same top status row; its 18 px mode/play icons sit below at y=24.
- The card shadow carries the battery/charging colour; the status markers do not.

### Now Playing

- Landscape: title at (8, 8), Bluetooth block at x=251, state icons at x=269/294, spectrum
  304×70, progress at y=138.
- Portrait: Bluetooth block at x=101, state icons at x=119/144, title at (8, 24), spectrum
  154×194 at y=86, progress at y=288.
- The time row and progress thumb are removed. The bottom area is empty except for the transient
  volume number while changing volume.

### Sorting

- GB2312 Chinese sorts by primary pinyin and kana by romaji, interleaved with Latin under
  A-Z. Unknown Hanzi/kanji keep their code point and fall under the final `#`.
- The asset is `assets/pinyin.bin`; sorting uses the rescan scratch, not daily RAM. The
  selected row's section is read from the image on demand.
- Long Menu in a lettered list opens the A-Z/# jump overlay; volume selects, Play jumps.
  Volume hold only scrolls rows. There is no double Menu.

### Now Playing seek feedback

Seek uses the progress and time display; there is no separate hold gauge. A 28×8 hold gauge
was prototyped and deferred because it duplicated that feedback and hid the Bluetooth block.

### Settings

- Settings home: five cards with 18/20 px colour icons — Random mode, Sleep timer, Sound,
  Display, System. Landscape cards are 304×25 at y=29 with 2 px gaps; portrait cards are
  154×48 at y=58 with 3 px gaps. Selected cards get the selection fill, border and white icons.
- Random and Sleep are direct items; Sound/Display/System open category pages.
- Category pages: no text header landscape; the half-badge and glyph identify the category.
  Portrait keeps the category title and colour rule. Category dots are removed.
- Categories: Sound (6 rows), Display (6 rows), System (10 rows).

### Sleep timer

The sleep timer remains a **shutdown** timer; this pass does not add a pause mode. Implemented.

- Panel: the moon pill shows the remaining time (`14:32`, `0:45`), updated every second; when
  the deadline passes while a track is playing, it shows `END` while waiting for that track.
- At the deadline, playing with a track: wait for the natural end of the current track, then
  power off. Do not start the next track audibly.
- Paused, idle, or no track at the deadline: power off immediately.
- Manual next/previous counts as the current track ending; repeat-one's loop point counts as
  the end. Seeking within the track does not.
- Pausing during the `END` wait powers off immediately.
- The timer lives in RAM; a restart or power loss cancels it. There is no RTC alarm.

![Sleep timer countdown](../images/lite/sleep-timer-countdown.png)

### Lists

- Landscape: five rows.
- Portrait: nine rows of 28 px in a 154×272 card at (8, 40); the card ends 8 px above the
  bottom edge.

### Boot / charge

- Boot landscape: 200×112 still at (60, 29) with a 5 px hard shadow; no card panel, no frame.
- Boot portrait: 146×112 still at (12, 104) with a 5 px hard shadow; no card panel, no frame.
- Charge: 200×112 landscape card, 146×112 portrait card, 96×64 animation inside.
- Boot art: Mountain Dusk by ansimuz, CC0 1.0.

## Selection and status

- Selection is flat blue `#006FEE` with white text.
- Normal dark card shadow is `#000000`; the dark border stays `#52525B`.
- Low battery / charging status colours override the card shadow: yellow ≤20%, purple ≤10%,
  red ≤5%, green when charging.
- Dark mode keeps light borders only on badges; large surfaces use 1 px grey borders to avoid
  a wall of white lines at night.
