from __future__ import annotations

import argparse
import struct
import subprocess
from collections import Counter
from pathlib import Path

MAGIC = b"LBO2"
WIDTH = 166
HEIGHT = 316
HEADER_BYTES = 48
PALETTE_COLORS = 16


def rgb565(red: int, green: int, blue: int) -> int:
    return ((red * 31 + 127) // 255) << 11 | ((green * 63 + 127) // 255) << 5 | \
           ((blue * 31 + 127) // 255)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path, help="Mountain Dusk preview PNG")
    parser.add_argument("--asset", type=Path,
                        default=Path(__file__).resolve().parents[1] / "assets/lite_animation.bin")
    args = parser.parse_args()

    raw = subprocess.run(
        ["magick", str(args.source), "-gravity", "center", "-crop", "126x240+0+0", "+repage",
         "-resize", f"{WIDTH}x{HEIGHT}!", "-colors", str(PALETTE_COLORS), "-depth", "8",
         "rgb:-"],
        check=True, capture_output=True).stdout
    if len(raw) != WIDTH * HEIGHT * 3:
        raise SystemExit(f"expected {WIDTH * HEIGHT * 3} RGB bytes, got {len(raw)}")
    pixels = [tuple(raw[index:index + 3]) for index in range(0, len(raw), 3)]
    counts = Counter(pixels)
    if len(counts) > PALETTE_COLORS:
        raise SystemExit(f"{len(counts)} colours after quantising; need <= {PALETTE_COLORS}")
    palette = [colour for colour, _ in counts.most_common(PALETTE_COLORS)]
    while len(palette) < PALETTE_COLORS:
        palette.append((0, 0, 0))
    index_of = {colour: index for index, colour in enumerate(palette)}

    blob = bytearray(MAGIC + bytes([1, 0]))
    blob += struct.pack("<HH", WIDTH, HEIGHT)
    blob += b"\0\0"
    for red, green, blue in palette:
        blob += struct.pack("<H", rgb565(red, green, blue))
    blob += b"\0\0\0\0"
    for row in range(HEIGHT):
        base = row * WIDTH
        for x in range(0, WIDTH, 2):
            high = index_of[pixels[base + x]]
            low = index_of[pixels[base + x + 1]]
            blob.append(high << 4 | low)
    if len(blob) != HEADER_BYTES + WIDTH * HEIGHT // 2:
        raise SystemExit(f"unexpected portrait blob size {len(blob)}")

    asset = args.asset.read_bytes()
    if MAGIC in asset:
        raise SystemExit("the asset already holds an LBO2 portrait still")
    args.asset.write_bytes(asset + blob)
    print(f"appended {len(blob):,} bytes at {len(asset):,}; asset is now {len(asset) + len(blob):,}")


if __name__ == "__main__":
    main()
