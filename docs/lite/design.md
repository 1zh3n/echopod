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
| card and badge shadow | `#0A0A0A` | `#52525B` |
| selection | `#006FEE` | `#006FEE` |

Badge colours: blue `#2FA8FF`, orange `#FF9F1C`, purple `#8B5CF6`, indigo `#6366F1`,
cyan `#35C7E8`, amber `#F59E0B`, success `#22C55E`.

## Layout

### Home

- Landscape: 104×104 card, 64 px selected icon, 40 px side icons.
- Portrait: 136×136 card at (17, 96), 80 px selected icon, 40 px side icons above/below; 20 px gap to the card. Status icons sit in a top row; the playing equalizer is top right.
- No text, no page dots. Equalizer appears only while playing.

### Home status

- Top-left 10×18 blue Bluetooth rune: connected; hidden when disconnected. Same visual height as the playback bars.
- Top-right four bars: playing indicator; hidden when paused or stopped. It is not a button.
- The card shadow carries the battery/charging colour; the Bluetooth rune and bars do not.
- The Bluetooth rune is drawn at 2× scale, so the shape stays readable at 1x. The old mockup drew a tiny boxed glyph and an unboxed white rune; both are replaced.

### Now Playing

- Landscape: title at (16, 8), state icons at x=262/288, spectrum 288×75.
- Portrait: state icons at x=122/147, title below, spectrum 146×158, progress at y=262.

### Settings

- Landscape: half badge at the left, five 26 px rows.
- Portrait: no large icon; category title, colour rule, two-line rows.
  Sound 6×38 px, System 7×34 px in a 154×244 card.

### Quick panel

- Landscape: two 140×112 cards side by side.
- Portrait: two 146×118 cards stacked at y=35/167.

### Lists

- Landscape: five rows.
- Portrait: nine rows of 28 px.

### Boot / charge

- Boot landscape: 316×166 still in a 2 px frame.
- Boot portrait: 166×316 still in a 2 px frame.
- Charge: 200×112 landscape card, 146×112 portrait card, 96×64 animation inside.
- Boot art: Mountain Dusk by ansimuz, CC0 1.0.

## Selection and status

- Selection is flat blue `#006FEE` with white text.
- Normal dark card shadow is `#52525B`.
- Low battery / charging status colours override the card shadow: yellow ≤20%, purple ≤10%,
  red ≤5%, green when charging.
- Dark mode keeps light borders only on badges; large surfaces use 1 px grey borders to avoid
  a wall of white lines at night.
