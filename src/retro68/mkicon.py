#!/usr/bin/env python3
# Turns a PNG (8-bit RGB or RGBA, not interlaced, square) into the icon
# family of a classic Mac OS application, as Rez source:
#
#   'ICN#' 32x32 black & white icon + mask    'ics#' the same at 16x16
#   'icl8' 32x32, 256 colors                  'ics8' the same at 16x16
#   'icl4' 32x32, 16 colors                   'ics4' the same at 16x16
#
# The 8-bit icons use the Macintosh system palette, the 4-bit ones the
# standard 16-color palette. Transparency comes from the PNG alpha channel
# or, without one, from the white background around the picture (a flood
# fill from the border).
#
# Usage: mkicon.py input.png output.r [resource id, default 128]
#
# Only the standard library is used.

import struct
import sys
import zlib


def read_png(path):
    with open(path, 'rb') as f:
        data = f.read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError(f'{path}: not a PNG file')
    pos = 8
    idat = b''
    width = height = depth = ctype = interlace = None
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IHDR':
            width, height, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', chunk)
        elif kind == b'IDAT':
            idat += chunk
        elif kind == b'IEND':
            break
    if depth != 8 or ctype not in (2, 6) or interlace != 0:
        raise ValueError(f'{path}: only 8-bit RGB / RGBA PNGs without interlacing are supported')
    bpp = 3 if ctype == 2 else 4
    raw = zlib.decompress(idat)
    stride = width * bpp
    rows = []
    prev = bytearray(stride)
    pos = 0
    for _ in range(height):
        ftype = raw[pos]
        line = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if ftype == 1:
                line[i] = (line[i] + a) & 0xff
            elif ftype == 2:
                line[i] = (line[i] + b) & 0xff
            elif ftype == 3:
                line[i] = (line[i] + ((a + b) >> 1)) & 0xff
            elif ftype == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                line[i] = (line[i] + pred) & 0xff
            elif ftype != 0:
                raise ValueError(f'{path}: bad filter type {ftype}')
        rows.append(bytes(line))
        prev = line
    pixels = []
    for y in range(height):
        row = rows[y]
        pixels.append([tuple(row[x * bpp:x * bpp + 3]) + ((row[x * bpp + 3],) if bpp == 4 else (255,))
                       for x in range(width)])
    return width, height, bpp == 4, pixels


def opacity(width, height, has_alpha, pixels):
    """1.0 for the picture, 0.0 for the background"""
    if has_alpha:
        return [[p[3] / 255.0 for p in row] for row in pixels]
    # white (nearly) background connected to the border
    def is_bg(p):
        return p[0] >= 240 and p[1] >= 240 and p[2] >= 240
    bg = [[False] * width for _ in range(height)]
    stack = [(x, y) for x in range(width) for y in (0, height - 1)] + \
            [(x, y) for y in range(height) for x in (0, width - 1)]
    while stack:
        x, y = stack.pop()
        if bg[y][x] or not is_bg(pixels[y][x]):
            continue
        bg[y][x] = True
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < width and 0 <= ny < height and not bg[ny][nx]:
                stack.append((nx, ny))
    return [[0.0 if bg[y][x] else 1.0 for x in range(width)] for y in range(height)]


def scale(width, height, pixels, alpha, size):
    """box filter down to size x size: (r, g, b, coverage) per pixel; the
    color only averages the opaque part (no white fringe)"""
    out = []
    for oy in range(size):
        y0, y1 = oy * height // size, (oy + 1) * height // size
        row = []
        for ox in range(size):
            x0, x1 = ox * width // size, (ox + 1) * width // size
            r = g = b = w = 0.0
            n = 0
            for y in range(y0, y1):
                for x in range(x0, x1):
                    a = alpha[y][x]
                    p = pixels[y][x]
                    r += p[0] * a
                    g += p[1] * a
                    b += p[2] * a
                    w += a
                    n += 1
            if w > 0:
                row.append((r / w, g / w, b / w, w / n))
            else:
                row.append((255.0, 255.0, 255.0, 0.0))
        out.append(row)
    return out


def system_palette():
    """the 256 colors of the Macintosh system palette"""
    levels = [0xFF, 0xCC, 0x99, 0x66, 0x33, 0x00]
    pal = []
    for r in levels:
        for g in levels:
            for b in levels:
                pal.append((r, g, b))
    pal = pal[:215]   # the cube without black (it is entry 255)
    ramp = [0xEE, 0xDD, 0xBB, 0xAA, 0x88, 0x77, 0x55, 0x44, 0x22, 0x11]
    pal += [(v, 0, 0) for v in ramp]
    pal += [(0, v, 0) for v in ramp]
    pal += [(0, 0, v) for v in ramp]
    pal += [(v, v, v) for v in ramp]
    pal.append((0, 0, 0))
    return pal


PALETTE16 = [
    (0xFF, 0xFF, 0xFF), (0xFC, 0xF3, 0x05), (0xFF, 0x64, 0x02), (0xDD, 0x08, 0x06),
    (0xF2, 0x08, 0x84), (0x46, 0x00, 0xA5), (0x00, 0x00, 0xD4), (0x02, 0xAB, 0xEA),
    (0x1F, 0xB7, 0x14), (0x00, 0x64, 0x11), (0x56, 0x2C, 0x05), (0x90, 0x71, 0x3A),
    (0xC0, 0xC0, 0xC0), (0x80, 0x80, 0x80), (0x40, 0x40, 0x40), (0x00, 0x00, 0x00),
]


def nearest(pal, c):
    best, bi = None, 0
    for i, p in enumerate(pal):
        # weighted distance (the eye is most sensitive to green)
        d = 3 * (p[0] - c[0]) ** 2 + 4 * (p[1] - c[1]) ** 2 + 2 * (p[2] - c[2]) ** 2
        if best is None or d < best:
            best, bi = d, i
    return bi


def indexed(img, pal, size):
    out = []
    for y in range(size):
        for x in range(size):
            r, g, b, a = img[y][x]
            out.append(nearest(pal, (r, g, b)) if a >= 0.5 else 0)
    return out


def mask_bits(img, size):
    bits = bytearray(size * size // 8)
    for y in range(size):
        for x in range(size):
            if img[y][x][3] >= 0.5:
                bits[(y * size + x) // 8] |= 0x80 >> (x % 8)
    return bytes(bits)


def bw_bits(img, size):
    """black & white: a plain threshold on the luminance (the dark parts of
    the picture black, the light ones white), the outline black"""
    inside = [[img[y][x][3] >= 0.5 for x in range(size)] for y in range(size)]
    bits = bytearray(size * size // 8)
    for y in range(size):
        for x in range(size):
            if not inside[y][x]:
                continue
            r, g, b, _ = img[y][x]
            lum = 0.30 * r + 0.59 * g + 0.11 * b
            edge = any(not (0 <= x + dx < size and 0 <= y + dy < size and inside[y + dy][x + dx])
                       for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if edge or lum < 150:
                bits[(y * size + x) // 8] |= 0x80 >> (x % 8)
    return bytes(bits)


def pack4(idx):
    return bytes((idx[i] << 4) | idx[i + 1] for i in range(0, len(idx), 2))


def rez_data(kind, rid, data, name=''):
    hexs = data.hex().upper()
    lines = []
    for i in range(0, len(hexs), 64):
        lines.append('\t$"' + hexs[i:i + 64] + '"')
    label = f', "{name}"' if name else ''
    return f"data '{kind}' ({rid}{label}) {{\n" + '\n'.join(lines) + '\n};\n'


def main(argv):
    if len(argv) not in (3, 4):
        print(__doc__ if __doc__ else 'usage: mkicon.py input.png output.r [id]', file=sys.stderr)
        return 2
    rid = int(argv[3]) if len(argv) == 4 else 128
    width, height, has_alpha, pixels = read_png(argv[1])
    alpha = opacity(width, height, has_alpha, pixels)
    pal8 = system_palette()

    out = ['/* generated by mkicon.py from ' + argv[1].split('/')[-1] + ': do not edit */\n']
    for size, bw, c8, c4 in ((32, "ICN#", "icl8", "icl4"), (16, "ics#", "ics8", "ics4")):
        img = scale(width, height, pixels, alpha, size)
        out.append(rez_data(bw, rid, bw_bits(img, size) + mask_bits(img, size)))
        out.append(rez_data(c8, rid, bytes(indexed(img, pal8, size))))
        out.append(rez_data(c4, rid, pack4(indexed(img, PALETTE16, size))))

    with open(argv[2], 'w') as f:
        f.write('\n'.join(out))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
