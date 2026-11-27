import argparse
import struct
import subprocess
from collections import Counter
from pathlib import Path

HEADER_BYTES = 48
PALETTE_COLORS = 16
LANDSCAPE = ("LBOO", 196, 108, "196x108+62+0")
PORTRAIT = ("LBO2", 142, 108, "142x108+89+0")


def rgb565(red, green, blue):
    return ((red * 31 + 127) // 255) << 11 | ((green * 63 + 127) // 255) << 5 | \
           ((blue * 31 + 127) // 255)


def still(source, width, height, crop):
    raw = subprocess.run(
        ["magick", str(source), "-crop", crop, "+repage", "-colors", str(PALETTE_COLORS),
         "-depth", "8", "rgb:-"],
        check=True, capture_output=True).stdout
    if len(raw) != width * height * 3:
        raise SystemExit(f"expected {width * height * 3} RGB bytes, got {len(raw)}")
    pixels = [tuple(raw[index:index + 3]) for index in range(0, len(raw), 3)]
    counts = Counter(pixels)
    if len(counts) > PALETTE_COLORS:
        raise SystemExit(f"{len(counts)} colours after quantising; need <= {PALETTE_COLORS}")
    palette = [colour for colour, _ in counts.most_common(PALETTE_COLORS)]
    while len(palette) < PALETTE_COLORS:
        palette.append((0, 0, 0))
    index_of = {colour: index for index, colour in enumerate(palette)}
    data = bytearray()
    for row in range(height):
        base = row * width
        for x in range(0, width, 2):
            data.append(index_of[pixels[base + x]] << 4 | index_of[pixels[base + x + 1]])
    return palette, bytes(data)


def section(magic, width, height, palette, data):
    out = bytearray(magic + bytes([1, 0]))
    out += struct.pack("<HH", width, height)
    out += b"\0\0"
    for red, green, blue in palette:
        out += struct.pack("<H", rgb565(red, green, blue))
    out += b"\0\0\0\0" + data
    return bytes(out)


def animation_end(asset):
    if asset[:4] != b"LANI" or asset[4] != 1:
        raise SystemExit("lite_animation.bin has no LANI header")
    frames = asset[5]
    width = asset[6]
    height = asset[7]
    end = 40 + frames * width * height // 2
    if end > len(asset):
        raise SystemExit("LANI section runs past the end of the asset")
    return end


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("--asset", type=Path,
                        default=Path(__file__).resolve().parents[1] / "assets/lite_animation.bin")
    args = parser.parse_args()
    asset = args.asset.read_bytes()
    prefix = asset[:animation_end(asset)]
    land_palette, land_data = still(args.source, LANDSCAPE[1], LANDSCAPE[2], LANDSCAPE[3])
    port_palette, port_data = still(args.source, PORTRAIT[1], PORTRAIT[2], PORTRAIT[3])
    out = prefix + section(b"LBOO", LANDSCAPE[1], LANDSCAPE[2], land_palette, land_data)
    out += section(b"LBO2", PORTRAIT[1], PORTRAIT[2], port_palette, port_data)
    args.asset.write_bytes(out)
    print(f"animation {len(prefix):,}; landscape {len(land_data) + HEADER_BYTES:,}; "
          f"portrait {len(port_data) + HEADER_BYTES:,}; total {len(out):,}")


if __name__ == "__main__":
    main()
