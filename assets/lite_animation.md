# Lite boot still + charging animation asset

`lite_animation.bin` is a 313,300-byte blob included in the Lite image as an extra glyph-table entry.
It starts with the charging animation, then the landscape boot still, then the portrait boot still.

## Charging animation

```text
offset  size  field
0       4     "LANI"
4       1     version = 1
5       1     frame count = 96
6       1     width = 96
7       1     height = 64
8       32    16 x RGB565 palette, little-endian
40      294912 96 frames x 96x64 x 4-bit (high nibble first)
```

The runtime streams the header and two 3,072-byte frame buffers through the 8,192-byte Lite
animation arena; it does not load all 96 frames at once.

## Landscape static boot picture

```text
offset  size  field
294952  4     "LBOO"
294956  1     version = 1
294957  1     reserved
294958  2     width = 196 (little-endian)
294960  2     height = 108 (little-endian)
294962  2     reserved
294964  32    16 x RGB565 palette, little-endian
294996  4     reserved
295000  10584 196x108 x 4-bit (high nibble first)
```

The 196x108 landscape art is drawn at (62, 31) inside a 2px frame, giving the 200x112 card
footprint at (60, 29) with a 5px hard shadow. There is no outer card panel. It is static; the
charging screen continues to use the 96x64 animation card.

## Portrait static boot picture

```text
offset 305584  size  field
305584  4     "LBO2"
305588  1     version = 1
305589  1     reserved
305590  2     width = 142 (little-endian)
305592  2     height = 108 (little-endian)
305594  2     reserved
305596  32    16 x RGB565 palette, little-endian
305628  4     reserved
305632  7668  142x108 x 4-bit (high nibble first)
```

The 142x108 portrait art is drawn at (14, 106) inside a 2px frame, giving the 146x112 card
footprint at (12, 104) with a 5px hard shadow. There is no outer card panel.

`tools/lite_boot_stills.py` regenerates both stills from the Mountain Dusk preview: landscape
crop 196x108 at +62+0; portrait crop 142x108 at +89+0; both quantized to 16 colours. Source and
license: Mountain Dusk by ansimuz, CC0 1.0.
