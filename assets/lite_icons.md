# Lite icon glyph face

`lite_icons.bin` is a 16,930-byte glyph-table entry used only by the Lite build. It holds the
Lite icon masks that used to live in the code window as `static const` arrays. The masks are
4-bit alpha levels, high nibble first; every icon starts on a byte boundary.

The C side points at each icon with an encoded offset (`LITE_ICON_POINTER(offset)`) instead of a
RAM address. `src/ui/lite_icons.c` decodes the offset, streams the mask through a four-slot
2,048-byte cache, and draws from the cache. `tools/lite_icon_asset.py` rebuilds this file from the
`#ifndef ECHO_LITE` arrays in the UI sources; run it with `--check` to verify the committed file.

| Offset | Bytes | Size | Icon |
|---:|---:|---:|---|
| 0 | 0 | 64×64 | Home ALL |
| 2,048 | 2,048 | 64×64 | Home LIST |
| 4,096 | 2,048 | 64×64 | Home GEAR |
| 6,144 | 2,048 | 64×64 | Home shadow |
| 8,192 | 1,152 | 48×48 | shuffle |
| 9,344 | 1,152 | 48×48 | loop |
| 10,496 | 1,152 | 48×48 | loop one |
| 11,648 | 1,152 | 48×48 | moon |
| 12,800 | 162 | 18×18 | shuffle |
| 12,962 | 162 | 18×18 | loop |
| 13,124 | 162 | 18×18 | loop one |
| 13,286 | 162 | 18×18 | play |
| 13,448 | 162 | 18×18 | pause |
| 13,610 | 800 | 40×40 | Sound category |
| 14,410 | 800 | 40×40 | Display category |
| 15,210 | 800 | 40×40 | Library category |
| 16,010 | 800 | 40×40 | Buttons category |
| 16,810 | 120 | 12×20 | Bluetooth status |

The face is appended after `lite_animation.bin` in the Lite glyph table. The image builder places
faces largest first, so the table offset changes when assets change; only the entry order and the
relative offsets inside this file are stable.
