import argparse
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FILES = [
    ROOT / "src/ui/lite_color_icons.c",
    ROOT / "src/ui/lite_mode_icons.c",
    ROOT / "src/ui/lite_category_icons.c",
]
ORDER = [
    "ALL_LEVELS", "LIST_LEVELS", "GEAR_LEVELS", "SHADOW_LEVELS",
    "shuffle_levels", "loop_levels", "loop_one_levels", "moon_levels",
    "shuffle18_levels", "loop18_levels", "loop_one18_levels", "play18_levels",
    "pause18_levels", "sound_levels", "display_levels", "library_levels",
    "buttons_levels",
]


def arrays(text):
    found = {}
    for match in re.finditer(r"static const uint8_t (\w+)\[(\d+)\] = \{(.*?)\n\};", text, re.S):
        data = bytes(int(value, 16) for value in re.findall(r"0x([0-9A-Fa-f]{2})", match.group(3)))
        if len(data) != int(match.group(2)):
            raise SystemExit(f"{match.group(1)} has {len(data)} bytes, declared {match.group(2)}")
        found[match.group(1)] = data
    return found


def build():
    all_arrays = {}
    for path in FILES:
        for name, data in arrays(path.read_text()).items():
            if name in all_arrays:
                raise SystemExit(f"duplicate array {name}")
            all_arrays[name] = data
    output = bytearray()
    offsets = []
    for name in ORDER:
        data = all_arrays.get(name)
        if data is None:
            raise SystemExit(f"missing {name}")
        offsets.append((name, len(output), len(data)))
        output.extend(data)
    return bytes(output), offsets


def main():
    parser = argparse.ArgumentParser(description="Pack the Lite icon masks into one glyph face")
    parser.add_argument("--check", action="store_true",
                        help="fail if assets/lite_icons.bin differs from a fresh build")
    arguments = parser.parse_args()
    output, offsets = build()
    target = ROOT / "assets/lite_icons.bin"
    if arguments.check:
        if not target.exists() or target.read_bytes() != output:
            raise SystemExit(f"out of date: {target}")
        print(f"lite_icons.bin up to date, {len(output)} bytes, {len(offsets)} icons")
        return
    target.write_bytes(output)
    for name, offset, size in offsets:
        print(f"{name} {offset} {size}")
    print(f"total {len(output)}")


if __name__ == "__main__":
    main()
