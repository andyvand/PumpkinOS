#!/usr/bin/env python3
# Builds a PlayStation 3 disc image (ISO9660 with a Joliet supplementary
# volume, the layout makeps3iso / genps3iso write) from a directory holding
# PS3_DISC.SFB and PS3_GAME/, or lists the directories of an image. Used by
# GNUMakefile for bin/PumpkinOS_PS3.iso (no makeps3iso on this host; based
# on src/ps2/mkiso.py).
#
# The primary volume carries upper case level 2 names with ";1", the Joliet
# volume the real names (what the console and RPCS3 use). The system area
# (sectors 0-15) starts with the PS3 disc header:
#
#   sector 0: the region table, big endian: the number of plain regions (1:
#             the image is not encrypted), a reserved word, then the first
#             and the last sector of the region (0 and the last sector of
#             the image)
#   sector 1: "PlayStation3" (16 bytes, zero padded) and the title id with
#             a dash ("PUMP-00002"), padded with blanks to 32 bytes

import argparse
import os
import struct
import sys
import time

SECTOR = 2048


def both(n, size):
    # little endian then big endian ("both byte orders")
    if size == 4:
        return struct.pack('<I', n) + struct.pack('>I', n)
    return struct.pack('<H', n) + struct.pack('>H', n)


def pad(data, size, fill=b' '):
    # (fill may be more than one byte: the UCS-2 space of the Joliet strings)
    return data[:size] + (fill * size)[:max(0, size - len(data))]


def sectors(size):
    return (size + SECTOR - 1) // SECTOR


def iso_name(name, is_dir, used):
    # ISO9660 level 2: up to 31 characters of A-Z 0-9 _ (and one dot for a
    # file), unique in the directory
    base, ext = (name, '') if is_dir else os.path.splitext(name)
    clean = lambda s: ''.join(c if c.isalnum() or c == '_' else '_' for c in s.upper())
    base, ext = clean(base), clean(ext[1:])
    if is_dir:
        limit = 31
        candidate = base[:limit] or '_'
    else:
        limit = 31 - 1 - len(ext)
        candidate = (base[:limit] or '_') + '.' + ext
    n = 1
    result = candidate
    while result in used:
        suffix = str(n)
        n += 1
        if is_dir:
            result = candidate[:limit - len(suffix)] + suffix
        else:
            result = candidate.split('.')[0][:limit - len(suffix)] + suffix + '.' + ext
    used.add(result)
    return result if is_dir else result + ';1'


def joliet_name(name, is_dir):
    name = name[:64]
    if not is_dir:
        name = name[:62] + ';1'
    return name.encode('utf-16-be')


class Entry:
    def __init__(self, path, name, parent):
        self.path = path
        self.name = name
        self.parent = parent
        self.is_dir = os.path.isdir(path)
        self.size = 0 if self.is_dir else os.path.getsize(path)
        self.children = []
        self.iso = None      # name in the primary volume (bytes)
        self.jol = None      # name in the Joliet volume (bytes)
        self.lba = 0         # extent of the file, or of the primary directory
        self.jlba = 0        # extent of the Joliet directory
        self.dirsize = 0     # size of the primary directory extent
        self.jdirsize = 0
        self.number = 0      # directory number in the path tables
        self.jnumber = 0


def scan(root):
    top = Entry(root, '', None)
    top.iso = b'\x00'
    top.jol = b'\x00'
    dirs = [top]
    i = 0
    while i < len(dirs):
        d = dirs[i]
        i += 1
        used = set()
        names = sorted(n for n in os.listdir(d.path) if not n.startswith('.'))
        for n in names:
            e = Entry(os.path.join(d.path, n), n, d)
            e.iso = iso_name(n, e.is_dir, used).encode('ascii')
            e.jol = joliet_name(n, e.is_dir)
            d.children.append(e)
            if e.is_dir:
                dirs.append(e)
    return top, dirs


def date7(t):
    tm = time.gmtime(t)
    return bytes([tm.tm_year - 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0])


def date17(t):
    return time.strftime('%Y%m%d%H%M%S00', time.gmtime(t)).encode('ascii') + b'\x00'


def record(name, lba, size, is_dir, t):
    length = 33 + len(name)
    if length % 2:
        length += 1
    r = bytes([length, 0]) + both(lba, 4) + both(size, 4) + date7(t)
    r += bytes([2 if is_dir else 0, 0, 0]) + both(1, 2) + bytes([len(name)]) + name
    return pad(r, length, b'\x00')


def directory(d, t, joliet):
    # the records of one directory: itself, its parent, its children sorted
    # by name; a record never crosses a sector boundary
    def attrs(e):
        if joliet:
            return e.jlba if e.is_dir else e.lba, e.jdirsize if e.is_dir else e.size
        return e.lba, e.dirsize if e.is_dir else e.size
    parent = d.parent or d
    data = record(b'\x00', *attrs(d), True, t) + record(b'\x01', *attrs(parent), True, t)
    key = (lambda e: e.jol) if joliet else (lambda e: e.iso)
    for e in sorted(d.children, key=key):
        r = record(key(e), *attrs(e), e.is_dir, t)
        if len(data) % SECTOR + len(r) > SECTOR:
            data = pad(data, sectors(len(data)) * SECTOR, b'\x00')
        data += r
    return pad(data, sectors(len(data)) * SECTOR, b'\x00')


def path_table(dirs, joliet, big):
    data = b''
    for d in dirs:
        name = d.jol if joliet else d.iso
        lba = d.jlba if joliet else d.lba
        parent = (d.parent or d).jnumber if joliet else (d.parent or d).number
        r = bytes([len(name), 0])
        r += struct.pack('>I' if big else '<I', lba) + struct.pack('>H' if big else '<H', parent)
        r += name
        if len(r) % 2:
            r += b'\x00'
        data += r
    return data


def descriptor(kind, volid, total, ptsize, ptl, ptm, root, t, publisher, joliet):
    if joliet:
        # UCS-2 strings, padded with UCS-2 spaces
        def enc(s, n):
            return pad(s.encode('utf-16-be'), n, b'\x00 ')
    else:
        def enc(s, n):
            return pad(s.encode('ascii'), n)
    d = bytes([kind]) + b'CD001' + bytes([1, 0])
    d += enc('PS3VOLUME', 32)
    d += enc(volid, 32)
    d += b'\x00' * 8
    d += both(total, 4)
    d += pad(b'%/E' if joliet else b'', 32, b'\x00')
    d += both(1, 2) + both(1, 2) + both(SECTOR, 2)
    d += both(ptsize, 4)
    d += struct.pack('<I', ptl) + struct.pack('<I', 0) + struct.pack('>I', ptm) + struct.pack('>I', 0)
    d += root
    d += enc(volid, 128) + enc(publisher, 128) + enc('', 128) + enc('PUMPKINOS MKISO', 128)
    d += enc('', 37) + enc('', 37) + enc('', 37)
    d += date17(t) + date17(t) + b'0' * 16 + b'\x00' + date17(t)
    d += bytes([1, 0])
    return pad(d, SECTOR, b'\x00')


def system_area(total, title_id):
    area = bytearray(16 * SECTOR)
    struct.pack_into('>IIII', area, 0, 1, 0, 0, total - 1)
    area[0x800:0x810] = pad(b'PlayStation3', 16, b'\x00')
    tid = title_id.upper()
    if len(tid) == 9 and '-' not in tid:
        tid = tid[:4] + '-' + tid[4:]
    area[0x810:0x830] = pad(tid.encode('ascii'), 32)
    return bytes(area)


def disc_sfb(title_id):
    # PS3_DISC.SFB: ".SFB", version 1.0, then 32 byte entries (a 16 byte key,
    # the offset and the length of its value, 8 reserved bytes) whose values
    # start at 0x200: HYBRID_FLAG "g" (a game disc) and TITLE_ID
    tid = title_id.upper()
    if len(tid) == 9 and '-' not in tid:
        tid = tid[:4] + '-' + tid[4:]
    sfb = bytearray(0x800)
    sfb[0:8] = b'.SFB' + struct.pack('>I', 0x00010000)
    for i, (key, off, value) in enumerate(((b'HYBRID_FLAG', 0x200, b'g'), (b'TITLE_ID', 0x210, tid.encode('ascii')))):
        e = 0x20 + i * 0x20
        sfb[e:e + 16] = pad(key, 16, b'\x00')
        struct.pack_into('>II', sfb, e + 16, off, 0x10)
        sfb[off:off + 16] = pad(value, 16, b'\x00')
    return bytes(sfb)


def build(root, out, volid, publisher, title_id):
    t = time.time()
    top, dirs = scan(root)
    for n, d in enumerate(dirs):
        d.number = n + 1
        d.jnumber = n + 1

    # layout: 16 system sectors, PVD, SVD, terminator, 4 path tables (one
    # sector each), the primary directories, the Joliet directories, the files
    lba = 19
    ptl, ptm, jptl, jptm = lba, lba + 1, lba + 2, lba + 3
    lba += 4
    for d in dirs:
        d.dirsize = len(directory(d, t, False))
        d.lba = lba
        lba += sectors(d.dirsize)
    for d in dirs:
        d.jdirsize = len(directory(d, t, True))
        d.jlba = lba
        lba += sectors(d.jdirsize)
    files = [e for d in dirs for e in d.children if not e.is_dir]
    for f in files:
        f.lba = lba
        lba += max(1, sectors(f.size))
    total = lba

    with open(out, 'wb') as fp:
        fp.write(system_area(total, title_id))
        pt = path_table(dirs, False, False)
        jpt = path_table(dirs, True, False)
        fp.write(descriptor(1, volid, total, len(pt), ptl, ptm, record(b'\x00', top.lba, top.dirsize, True, t), t, publisher, False))
        fp.write(descriptor(2, volid, total, len(jpt), jptl, jptm, record(b'\x00', top.jlba, top.jdirsize, True, t), t, publisher, True))
        fp.write(pad(bytes([255]) + b'CD001' + bytes([1]), SECTOR, b'\x00'))
        fp.write(pad(pt, SECTOR, b'\x00'))
        fp.write(pad(path_table(dirs, False, True), SECTOR, b'\x00'))
        fp.write(pad(jpt, SECTOR, b'\x00'))
        fp.write(pad(path_table(dirs, True, True), SECTOR, b'\x00'))
        for d in dirs:
            fp.write(directory(d, t, False))
        for d in dirs:
            fp.write(directory(d, t, True))
        for f in files:
            with open(f.path, 'rb') as src:
                data = src.read()
            fp.write(pad(data, max(1, sectors(len(data))) * SECTOR, b'\x00'))
        assert fp.tell() == total * SECTOR
    return total, len(files), len(dirs)


def listing(path):
    with open(path, 'rb') as fp:
        def read(lba, n):
            fp.seek(lba * SECTOR)
            return fp.read(n)

        def walk(lba, size, indent, joliet):
            data = read(lba, size)
            o = 0
            while o < len(data):
                length = data[o]
                if length == 0:
                    o = (o // SECTOR + 1) * SECTOR
                    continue
                e = data[o:o + length]
                elba, esize, flags, nl = struct.unpack('<I', e[2:6])[0], struct.unpack('<I', e[10:14])[0], e[25], e[32]
                name = e[33:33 + nl]
                o += length
                if name in (b'\x00', b'\x01'):
                    continue
                text = name.decode('utf-16-be') if joliet else name.decode('latin1')
                print('%s%s%s%s' % (indent, text, '/' if flags & 2 else '', '' if flags & 2 else '  %d' % esize))
                if flags & 2:
                    walk(elba, esize, indent + '  ', joliet)

        lba = 16
        while True:
            d = read(lba, SECTOR)
            kind = d[0]
            if d[1:6] != b'CD001' or kind == 255:
                break
            if kind in (1, 2):
                joliet = kind == 2
                volid = d[40:72].decode('utf-16-be' if joliet else 'latin1').strip(' \x00')
                print('%s volume "%s", %d sectors' % ('Joliet' if joliet else 'ISO9660', volid, struct.unpack('<I', d[80:84])[0]))
                root = d[156:190]
                walk(struct.unpack('<I', root[2:6])[0], struct.unpack('<I', root[10:14])[0], '  ', joliet)
            lba += 1


def main():
    ap = argparse.ArgumentParser(description='build or list a PlayStation 3 disc image (ISO9660 + Joliet)')
    ap.add_argument('-o', '--output', help='the image to write')
    ap.add_argument('-V', '--volume', default='PS3VOLUME', help='volume identifier (default PS3VOLUME)')
    ap.add_argument('-i', '--title-id', default='PUMP00002', help='title id of the disc header (default PUMP00002)')
    ap.add_argument('-p', '--publisher', default='', help='publisher identifier')
    ap.add_argument('-s', '--sfb', action='store_true', help='write PS3_DISC.SFB (for the title id) into the directory first')
    ap.add_argument('-t', '--list', action='store_true', help='list the directories of an existing image')
    ap.add_argument('path', help='the directory to put on the disc, or the image to list')
    args = ap.parse_args()

    if args.list:
        listing(args.path)
        return 0
    if not args.output:
        ap.error('-o is required to build an image')
    if not os.path.isdir(args.path):
        ap.error('%s is not a directory' % args.path)
    if args.sfb:
        with open(os.path.join(args.path, 'PS3_DISC.SFB'), 'wb') as fp:
            fp.write(disc_sfb(args.title_id))
    total, nfiles, ndirs = build(args.path, args.output, args.volume.upper()[:32], args.publisher, args.title_id)
    print('%s: %d files in %d directories, %d sectors (%.1f MB)' % (args.output, nfiles, ndirs, total, total * SECTOR / 1048576.0))
    return 0


if __name__ == '__main__':
    sys.exit(main())
