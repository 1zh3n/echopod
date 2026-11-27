# Lite boot still + charging animation asset

`lite_animation.bin` is a 314,424-byte blob included in the Lite image as an extra glyph-table entry.
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
294958  2     width = 200 (little-endian)
294960  2     height = 112 (little-endian)
294962  2     reserved
294964  32    16 x RGB565 palette, little-endian
294996  4     reserved
295000  11200 200x112 x 4-bit (high nibble first)
```

The 200x112 landscape still is drawn at (60, 29) with a 5px hard shadow. There is no card panel
and no frame. It is static; the charging screen continues to use the 96x64 animation card.

## Portrait static boot picture

```text
offset 306200  size  field
306200  4     "LBO2"
306204  1     version = 1
306205  1     reserved
306206  2     width = 146 (little-endian)
306208  2     height = 112 (little-endian)
306210  2     reserved
306212  32    16 x RGB565 palette, little-endian
306244  4     reserved
306248  8176  146x112 x 4-bit (high nibble first)
```

The 146x112 portrait still is drawn at (12, 104) with a 5px hard shadow. There is no card panel
and no frame.

`tools/lite_boot_stills.py` regenerates both stills from the Mountain Dusk preview: landscape
crop 200x112 at +60+0; portrait crop 146x112 at +87+0; both quantized to 16 colours. Source and
license: Mountain Dusk by ansimuz, CC0 1.0.
