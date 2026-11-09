#!/usr/bin/env python3
# Packs a vfs/ tree into the archive linked into pumpkinos_embed.dol (see
# GNUMakefile and extract_blob() in main.c): for every file, a one byte name
# length, the name relative to the tree root (with '/' separators), a big
# endian 32 bit size and the data; a zero byte ends the archive.
#
#   mkvfsblob.py -o vfsblob.bin <vfs dir> [<subdir> ...]
#
# Only the given subdirectories of the tree are packed (all of it when none
# is named).

import os
import struct
import sys


def main(argv):
    out = None
    args = []
    i = 1
    while i < len(argv):
        if argv[i] == '-o' and i + 1 < len(argv):
            out = argv[i + 1]
            i += 2
        else:
            args.append(argv[i])
            i += 1
    if out is None or not args:
        sys.stderr.write('usage: mkvfsblob.py -o <archive> <vfs dir> [<subdir> ...]\n')
        return 1

    root = args[0]
    subdirs = args[1:] or ['']
    blob = bytearray()
    count = 0
    for sub in subdirs:
        top = os.path.join(root, sub) if sub else root
        for dirpath, dirnames, filenames in os.walk(top):
            dirnames.sort()
            for name in sorted(filenames):
                if name.startswith('.'):
                    continue
                full = os.path.join(dirpath, name)
                rel = os.path.relpath(full, root).replace(os.sep, '/')
                data = open(full, 'rb').read()
                rel_b = rel.encode('utf-8')
                if len(rel_b) > 255:
                    sys.stderr.write('name too long: %s\n' % rel)
                    return 1
                blob += struct.pack('B', len(rel_b)) + rel_b + struct.pack('>I', len(data)) + data
                count += 1
    blob += b'\0'
    with open(out, 'wb') as f:
        f.write(blob)
    sys.stdout.write('%s: %d files, %d bytes\n' % (out, count, len(blob)))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
