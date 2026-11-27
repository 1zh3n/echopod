# Lite design

The final Lite design is a neo-brutalist interface with runtime Light/Dark and landscape/portrait.
The mockup sheet is [mockups/index.html](mockups/index.html).

> The Home, Now Playing, Settings and Quick panel images are design mockups, not device screenshots.
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

## Layout

### Home

- Landscape: 104×104 card at (108, 24), 6 px radius, 64 px selected icon, 40 px side icons.
- Portrait: 112×112 card at (29, 108), 6 px radius, 64 px selected icon, 40 px side icons above/below; 32 px gaps to the card.
- No text, no page dots. No playback bars; playback lives in the global status meter.

### Home status

- Top-left 28×8 status meter at (2, 2): 2 px border, 2 px hard shadow, two 12×4 cells.
  Left cell blue `#006FEE` when Bluetooth is connected; right cell cyan `#35C7E8` when
  playing. Inactive cells use the divider colour.
- Light border is ink; dark border is `#52525B`; the shadow is black in both themes.
- Now Playing replaces the meter with a 14×8 Bluetooth-only chip beside the shuffle/play
  icons (x=236/96, y=13); the play/pause icon already shows playback.
- The card shadow carries the battery/charging colour; the status meter does not.

### Now Playing

- Landscape: title at (16, 8), Bluetooth chip at x=236, state icons at x=262/288, spectrum 288×75.
- Portrait: Bluetooth chip at x=96, state icons at x=122/147, title below, spectrum 146×158,
  progress at y=262.

### Settings

- Landscape: no text header; the half-badge, category dots and rows carry the category. The
  status meter and the category dots share the top band at y=6; the card stays at y=28;
  five 26 px rows.
- Portrait: no large icon; category title, colour rule, two-line rows.
  Sound 6×38 px, System 7×34 px in a 154×244 card at y=58. The category title stays because
  portrait has no half-badge.

### Quick panel

- Landscape: two 140×112 cards side by side.
- Portrait: two 146×118 cards stacked at y=35/167.

### Lists

- Landscape: five rows.
- Portrait: nine rows of 28 px.

### Boot / charge

- Boot landscape: 196×108 art in a 2 px frame -> 200×112 card at (60, 29), 5 px shadow, no card panel.
- Boot portrait: 142×108 art in a 2 px frame -> 146×112 card at (12, 104), 5 px shadow, no card panel.
- Charge: 200×112 landscape card, 146×112 portrait card, 96×64 animation inside.
- Boot art: Mountain Dusk by ansimuz, CC0 1.0.

## Selection and status

- Selection is flat blue `#006FEE` with white text.
- Normal dark card shadow is `#000000`; the dark border stays `#52525B`.
- Low battery / charging status colours override the card shadow: yellow ≤20%, purple ≤10%,
  red ≤5%, green when charging.
- Dark mode keeps light borders only on badges; large surfaces use 1 px grey borders to avoid
  a wall of white lines at night.
