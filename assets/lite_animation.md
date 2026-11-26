# Lite boot/charging animation asset

`lite_animation.bin` is a 294,952-byte blob included in the Lite image as an extra glyph-table entry.

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
animation arena; it does not load all 96 frames at once. Source and license: `lite_animation.LICENSE`.
