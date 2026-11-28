import argparse
import json
import re
import shlex
import subprocess
import tempfile
from collections import defaultdict
from pathlib import Path

HEAP_SYMBOLS = {"malloc", "calloc", "realloc", "free"}
TARGET_NAMES = {
    "echo_app",
    "echo_core",
    "echo_device",
    "echo_dsp",
    "echo_fonts",
    "echo_gfx",
    "echo_image",
    "echo_library",
    "echo_library_scan",
    "echo_np_faces",
    "echo_playback",
    "echo_settings",
    "echo_sound",
    "echo_ui",
}


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Audit the static RAM of the Lite app from its compile commands"
    )
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--budget-bytes", type=int, required=True)
    parser.add_argument("--toolchain-prefix", default="arm-none-eabi-")
    return parser.parse_args()


def load_commands(path):
    entries = json.loads(path.read_text())
    selected = {}
    for entry in entries:
        source = Path(entry["file"])
        if "/src/src/" not in source.as_posix():
            continue
        match = re.search(r"CMakeFiles/([^/]+)\.dir", entry["command"])
        target = match.group(1) if match else ""
        if target not in TARGET_NAMES:
            continue
        selected.setdefault(source, entry["command"])
    return selected


def clean_command(command, output):
    parts = shlex.split(command)
    result = []
    skip_next = False
    for part in parts:
        if skip_next:
            skip_next = False
            continue
        if part == "-o":
            skip_next = True
            continue
        if part in {"-c", "-g", "-flto", "-ffat-lto-objects"} or part.startswith("-flto="):
            continue
        result.append(part)
    result += ["-fno-lto", "-o", str(output), "-c"]
    return result


def module_of(source):
    parts = source.parts
    for index in range(len(parts) - 2, -1, -1):
        if parts[index] == "src":
            return parts[index + 1]
    return source.parent.name


def run_output(command):
    return subprocess.run(command, check=True, capture_output=True, text=True).stdout


def main():
    arguments = parse_arguments()
    commands = load_commands(arguments.compile_commands)
    if not commands:
        raise SystemExit("no Lite src commands in compile_commands.json")
    size_tool = arguments.toolchain_prefix + "size"
    nm_tool = arguments.toolchain_prefix + "nm"
    ram_by_module = defaultdict(int)
    flash_by_module = defaultdict(int)
    heap_users = []
    with tempfile.TemporaryDirectory() as temporary:
        temporary = Path(temporary)
        for index, (source, command) in enumerate(sorted(commands.items())):
            object_file = temporary / f"{index}.o"
            subprocess.run(clean_command(command, object_file), check=True, capture_output=True,
                           text=True)
            sizes = run_output([size_tool, str(object_file)]).splitlines()[1].split()
            text, data, bss = (int(value) for value in sizes[:3])
            module = module_of(source)
            flash_by_module[module] += text + data
            ram_by_module[module] += data + bss
            for symbol in run_output([nm_tool, "--undefined-only", str(object_file)]).splitlines():
                name = symbol.split()[-1]
                if name in HEAP_SYMBOLS:
                    heap_users.append(f"{source}: {name}")

    total_flash = sum(flash_by_module.values())
    total_ram = sum(ram_by_module.values())
    print(f"{'module':<10}{'flash':>12}{'static RAM':>14}")
    for module in sorted(ram_by_module):
        print(f"{module:<10}{flash_by_module[module]:>12}{ram_by_module[module]:>14}")
    print(f"{'total':<10}{total_flash:>12}{total_ram:>14} of {arguments.budget_bytes} B RAM budget")
    if heap_users:
        print("\nheap users:")
        for line in heap_users:
            print(line)
    if total_ram > arguments.budget_bytes or heap_users:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
