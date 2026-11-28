# Final Lite mockups

These are the final design-reference sheets for the 512 MB Lite UI. The Home sheet is the pre-M8 design (Home was removed); Now Playing,
Settings and Quick panel sheets are **design mockups, not device screenshots**; real font
rendering and final asset placement can differ slightly. The Quick panel sheet is historical:
Quick is split into direct items on the Settings home. The boot sheet is rendered from the actual `LBOO` /
`LBO2` firmware assets.

The PNG files below are the design source. The HTML page is only a convenience index; it is not
a screenshot source.

## Final design rules

- Spacing: 4 px base; 8 px outer gutter on panel screens; 28 px landscape header.
- Home card: 6 px radius, 104×104 landscape / 112×112 portrait, 64 px selected icon, no keyline.
- Status: 28×8 meter at `(8, 4)` (blue = Bluetooth, cyan = playing). Now Playing uses a
  14×8 Bluetooth-only chip beside shuffle/play.
- Settings: landscape has no text header; the meter is at `(8, 4)` and the category dots centre
  on y=8. Portrait keeps the category name and ends its card 8 px above the bottom.
- Now Playing: 8 px sides/top/bottom; content width 304/154; times end 8 px above the bottom.
- Lists portrait: 154×272 card ending 8 px above the bottom.
- Settings Quick: intentional 12 px horizontal / 30–58 px vertical exception inside Settings.
- Battery/charging: still carried by the card shadow.
- Dark: border `#52525B`, shadow `#000000`.

## Home (historical; removed in M8)

![Home](../../images/lite/home-final-v6.png)

## Now Playing

![Now Playing](../../images/lite/now-playing-final-v3.png)

## Settings

![Settings](../../images/lite/settings-final-v4.png)

## Quick panel (historical; now Settings Quick)

![Quick panel](../../images/lite/quick-panel-final-v3.png)

## Sleep timer

Implemented. The moon pill shows the remaining time; after the deadline it shows `END` while
waiting for the current track, then the player shuts down. No pause mode.

![Sleep timer countdown](../../images/lite/sleep-timer-countdown.png)

## Boot stills

200×112 landscape / 146×112 portrait stills with a 5px hard shadow, no card or frame.

![Boot stills](../../images/lite/boot-shadow.png)

## Dark palette

```text
page       #0B0B0D
card       #17171C
text       #F4F4F5
muted      #A1A1AA
border     #52525B
shadow     #000000
selection  #006FEE
blue       #2FA8FF
orange     #FF9F1C
purple     #8B5CF6
indigo     #6366F1
```

The optional HTML index is [index.html](index.html). The implementation-level layout spec is
[../design.md](../design.md).
