#!/usr/bin/env python3
"""Generate 4-bit alpha masks for the Lite icon family.

The masks use the firmware pack order: even x in the high nibble,
odd x in the low nibble.  Shapes are supersampled 4x4 (48px) or 8x8
(18px) so the existing canvas alpha blending keeps the edges smooth.
"""
import math
import sys


def dist(px, py, ax, ay, bx, by):
    dx, dy = bx - ax, by - ay
    length2 = dx * dx + dy * dy
    if length2 == 0:
        return math.hypot(px - ax, py - ay)
    t = ((px - ax) * dx + (py - ay) * dy) / length2
    t = max(0.0, min(1.0, t))
    return math.hypot(px - (ax + t * dx), py - (ay + t * dy))


def segment(ax, ay, bx, by, width):
    return lambda x, y: dist(x, y, ax, ay, bx, by) <= width / 2


def polygon(points):
    def inside(x, y):
        hit = False
        j = len(points) - 1
        for i in range(len(points)):
            xi, yi = points[i]
            xj, yj = points[j]
            if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
                hit = not hit
            j = i
        return hit
    return inside


def circle(cx, cy, r):
    return lambda x, y: (x - cx) ** 2 + (y - cy) ** 2 <= r * r


def roundrect(x1, y1, x2, y2, r):
    def inside(x, y):
        if x < x1 or x > x2 or y < y1 or y > y2:
            return False
        cx = min(max(x, x1 + r), x2 - r)
        cy = min(max(y, y1 + r), y2 - r)
        return (x - cx) ** 2 + (y - cy) ** 2 <= r * r
    return inside


def star(cx, cy, r):
    pts = []
    for i in range(8):
        a = math.radians(i * 45)
        rad = r if i % 2 == 0 else r * 0.32
        pts.append((cx + rad * math.cos(a), cy + rad * math.sin(a)))
    return polygon(pts)


def union(*shapes):
    return lambda x, y: any(s(x, y) for s in shapes)


def subtract(base, cut):
    return lambda x, y: base(x, y) and not cut(x, y)


def loop_glyph(size):
    s = size / 48.0
    outer = roundrect(6 * s, 12 * s, 42 * s, 36 * s, 12 * s)
    inner = roundrect(11 * s, 17 * s, 37 * s, 31 * s, 7 * s)
    gap = lambda x, y: 18 * s <= x <= 30 * s and y <= 18 * s
    ring = lambda x, y: outer(x, y) and not inner(x, y)
    arrow = polygon([(32 * s, 13 * s), (24 * s, 8 * s), (24 * s, 18 * s)])
    return union(lambda x, y: ring(x, y) and not gap(x, y), arrow)


def loop_one_glyph(size):
    s = size / 48.0
    loop = loop_glyph(size)
    stem = polygon([(22.5 * s, 18 * s), (25.5 * s, 18 * s), (25.5 * s, 30 * s),
                    (22.5 * s, 30 * s)])
    flag = polygon([(22.5 * s, 18 * s), (18.5 * s, 20.2 * s), (22.5 * s, 20.8 * s)])
    return union(loop, stem, flag)


def moon_glyph(size):
    s = size / 48.0
    body = circle(24 * s, 24 * s, 17 * s)
    cut = circle(29 * s, 19 * s, 14.5 * s)
    crescent = lambda x, y: body(x, y) and not cut(x, y)
    return union(crescent, star(35 * s, 12 * s, 3.5 * s), star(40 * s, 20 * s, 2.5 * s))


def shuffle_glyph(size):
    s = size / 48.0
    parts = [
        segment(6 * s, 10 * s, 16 * s, 10 * s, 6 * s),
        segment(16 * s, 10 * s, 38 * s, 38 * s, 6 * s),
        segment(6 * s, 38 * s, 16 * s, 38 * s, 6 * s),
        segment(16 * s, 38 * s, 38 * s, 10 * s, 6 * s),
        polygon([(42 * s, 10 * s), (33 * s, 4 * s), (33 * s, 16 * s)]),
        polygon([(42 * s, 38 * s), (33 * s, 32 * s), (33 * s, 44 * s)]),
    ]
    return union(*parts)


def play_glyph(size):
    s = size / 48.0
    return polygon([(16 * s, 8 * s), (16 * s, 40 * s), (40 * s, 24 * s)])


def pause_glyph(size):
    s = size / 48.0
    return union(polygon([(14 * s, 8 * s), (22 * s, 8 * s), (22 * s, 40 * s), (14 * s, 40 * s)]),
                 polygon([(27 * s, 8 * s), (35 * s, 8 * s), (35 * s, 40 * s), (27 * s, 40 * s)]))


def coverage(shape, x, y, samples):
    hit = 0
    for sy in range(samples):
        for sx in range(samples):
            px = x + (sx + 0.5) / samples
            py = y + (sy + 0.5) / samples
            if shape(px, py):
                hit += 1
    return int(round(hit * 15.0 / (samples * samples)))


def make_mask(shape, size):
    samples = 8 if size <= 18 else 4
    values = []
    for y in range(size):
        for x in range(size):
            values.append(coverage(shape, x, y, samples))
    packed = []
    for y in range(size):
        row = values[y * size:(y + 1) * size]
        for x in range(0, size, 2):
            packed.append((row[x] << 4) | row[x + 1])
    return values, packed


def preview(values, size, name):
    scale = 8 if size <= 18 else 4
    path = f'/home/xia/Projects/echo-mini/.tmp/icon-test/{name}.ppm'
    with open(path, 'wb') as f:
        f.write(f'P6\n{size * scale} {size * scale}\n255\n'.encode())
        for y in range(size * scale):
            for x in range(size * scale):
                a = values[(y // scale) * size + (x // scale)]
                c = 255 if a > 0 else 0
                f.write(bytes((c, c, c)))
    print(f'preview {path}')


def c_array(name, packed):
    lines = []
    for i in range(0, len(packed), 16):
        lines.append('    ' + ', '.join(f'0x{v:02X}' for v in packed[i:i + 16]) + ',')
    return f'static const uint8_t {name}[{len(packed)}] = {{\n' + '\n'.join(lines) + '\n};'


def emit(kind):
    if kind == 'compact':
        for name, shape in [
            ('shuffle18_levels', shuffle_glyph(18)),
            ('loop18_levels', loop_glyph(18)),
            ('loop_one18_levels', loop_one_glyph(18)),
            ('play18_levels', play_glyph(18)),
            ('pause18_levels', pause_glyph(18)),
        ]:
            values, packed = make_mask(shape, 18)
            preview(values, 18, name)
            print(c_array(name, packed))
    elif kind == 'badge':
        for name, shape in [
            ('loop_levels', loop_glyph(48)),
            ('loop_one_levels', loop_one_glyph(48)),
            ('moon_levels', moon_glyph(48)),
        ]:
            values, packed = make_mask(shape, 48)
            preview(values, 48, name)
            print(c_array(name, packed))


if __name__ == '__main__':
    emit(sys.argv[1] if len(sys.argv) > 1 else 'compact')
