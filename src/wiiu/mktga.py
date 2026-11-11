#!/usr/bin/env python3
# Converts a PNG (8-bit RGB or RGBA, not interlaced) into the uncompressed
# TGA the Wii U menu reads from a title's meta/ directory:
#
#   iconTex.tga     128x128, 32 bits (-a)
#   bootTvTex.tga   1280x720, 24 bits
#   bootDrcTex.tga  854x480, 24 bits
#   bootLogoTex.tga 170x42, 32 bits (-a)
#
# Usage: mktga.py [-a] [-s WxH] input.png output.tga
#   -a     32 bits with alpha (24 bits otherwise)
#   -s     check that the image has this size
#
# Only the standard library is used (zlib for the PNG data).

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
        raise ValueError(f'{path}: only 8-bit RGB / RGBA PNGs without interlacing are supported '
                         f'(depth {depth}, color type {ctype}, interlace {interlace})')
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
    return width, height, bpp, rows


def write_tga(path, width, height, bpp, rows, alpha):
    out_bpp = 4 if alpha else 3
    # uncompressed true color, origin at the bottom left (rows bottom up),
    # pixels in BGR(A) order
    header = struct.pack('<BBBHHBHHHHBB', 0, 0, 2, 0, 0, 0, 0, 0, width, height,
                         out_bpp * 8, 8 if alpha else 0)
    body = bytearray()
    for row in reversed(rows):
        for x in range(width):
            px = row[x * bpp:(x + 1) * bpp]
            body += bytes((px[2], px[1], px[0]))
            if alpha:
                body.append(px[3] if bpp == 4 else 0xff)
    with open(path, 'wb') as f:
        f.write(header + bytes(body))


def main(argv):
    alpha = False
    size = None
    args = []
    i = 0
    while i < len(argv):
        if argv[i] == '-a':
            alpha = True
        elif argv[i] == '-s' and i + 1 < len(argv):
            w, h = argv[i + 1].lower().split('x')
            size = (int(w), int(h))
            i += 1
        else:
            args.append(argv[i])
        i += 1
    if len(args) != 2:
        sys.stderr.write('usage: mktga.py [-a] [-s WxH] input.png output.tga\n')
        return 1
    width, height, bpp, rows = read_png(args[0])
    if size and size != (width, height):
        sys.stderr.write(f'{args[0]}: {width}x{height}, expected {size[0]}x{size[1]}\n')
        return 1
    write_tga(args[1], width, height, bpp, rows, alpha)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
