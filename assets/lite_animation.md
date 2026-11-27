# Lite boot still + charging animation asset

`lite_animation.bin` is a 347,504-byte blob included in the Lite image as an extra glyph-table entry.
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

## Static boot picture

```text
offset  size  field
294952  4     "LBOO"
294956  1     version = 1
294957  1     reserved
294958  2     width = 316 (little-endian)
294960  2     height = 166 (little-endian)
294962  2     reserved
294964  32    16 x RGB565 palette, little-endian
294996  4     reserved
295000  26228 316x166 x 4-bit (high nibble first)
```

The landscape boot still is drawn at (2, 2) inside a 2px frame, filling the 320x170 screen. It is
static; the charging screen continues to use the 96x64 animation card.

## Portrait static boot picture

```text
offset 321228  size  field
321228  4     "LBO2"
321232  1     version = 1
321233  1     reserved
321234  2     width = 166 (little-endian)
321236  2     height = 316 (little-endian)
321238  2     reserved
321240  32    16 x RGB565 palette, little-endian
321272  4     reserved
321276  26228 166x316 x 4-bit (high nibble first)
```

The portrait still is appended after the landscape still. `tools/lite_boot_portrait.py` regenerates
it from the Mountain Dusk preview; the crop is 126x240, resized to 166x316 and quantized to 16
colors. Source and license: Mountain Dusk by ansimuz, CC0 1.0.
