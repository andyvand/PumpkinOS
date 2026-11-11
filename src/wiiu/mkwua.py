#!/usr/bin/env python3
# Packs directories into a Wii U Archive (.wua), the single file game image
# Cemu loads (File > Load, or a game path). A .wua is a ZArchive: the files'
# data concatenated and cut into 64KB blocks, each compressed with zstd, then
# the block offset table, the name table, the file tree and a footer with a
# SHA-256 of everything before it. The layout follows ZArchive's
# zarchivewriter.cpp (Cemu's dependencies/ZArchive) byte for byte.
#
# The archive root holds one directory per title, named
# <16 hex digit title id>_v<decimal version> (for example
# 0005000010F5A500_v0), each with the code/, content/ and meta/ directories
# of an installed title.
#
# Usage: mkwua.py -o out.wua dir [dir ...]
#   every dir is added under the archive root with its own name
#
# Needs Python 3.14 (compression.zstd) or the zstandard module.

import hashlib
import os
import struct
import sys

try:
    from compression import zstd as _zstd

    def zstd_compress(data):
        return _zstd.compress(data, level=6)
except ImportError:
    try:
        import zstandard as _zstandard

        def zstd_compress(data):
            return _zstandard.ZstdCompressor(level=6).compress(data)
    except ImportError:
        sys.stderr.write('mkwua.py: needs Python 3.14 (compression.zstd) or the zstandard module\n')
        sys.exit(1)

BLOCK_SIZE = 64 * 1024
ENTRIES_PER_OFFSETRECORD = 16
FOOTER_MAGIC = 0x169f52d6
FOOTER_VERSION1 = 0x61bf3a01
ROOT_NAME_OFFSET = 0x7FFFFFFF


class Node:
    def __init__(self, name, is_file):
        self.name = name
        self.is_file = is_file
        self.children = []
        self.offset = 0
        self.size = 0
        self.start_index = 0


def sort_key(name):
    # CompareNodeName: ASCII case-insensitive byte order, a prefix first
    return bytes(c + 32 if 65 <= c <= 90 else c for c in name.encode('utf-8'))


class Writer:
    def __init__(self, out):
        self.out = out
        self.sha = hashlib.sha256()
        self.written = 0
        self.block = bytearray()
        self.records = []      # [base offset, [size - 1, ...]]
        self.nblocks = 0
        self.input_offset = 0
        self.names = []
        self.name_index = {}
        self.root = Node('', False)

    def output(self, data):
        self.out.write(data)
        self.sha.update(data)
        self.written += len(data)

    def name_entry(self, name):
        if name not in self.name_index:
            self.name_index[name] = len(self.names)
            self.names.append(name)
        return self.name_index[name]

    def store_block(self, data):
        offset = self.written
        comp = zstd_compress(bytes(data))
        if len(comp) >= BLOCK_SIZE:
            comp = bytes(data)       # stored as is when it does not shrink
        self.output(comp)
        if self.nblocks % ENTRIES_PER_OFFSETRECORD == 0:
            self.records.append([offset, [0] * ENTRIES_PER_OFFSETRECORD])
        self.records[-1][1][self.nblocks % ENTRIES_PER_OFFSETRECORD] = len(comp) - 1
        self.nblocks += 1

    def append(self, data):
        mv = memoryview(data)
        while len(mv):
            n = min(BLOCK_SIZE - len(self.block), len(mv))
            self.block += mv[:n]
            mv = mv[n:]
            if len(self.block) == BLOCK_SIZE:
                self.store_block(self.block)
                self.block = bytearray()
        self.input_offset += len(data)

    def add_tree(self, parent, path, name):
        if os.path.isdir(path):
            node = Node(name, False)
            self.name_entry(name)
            parent.children.append(node)
            for entry in sorted(os.listdir(path)):
                if entry in ('.DS_Store',):
                    continue
                self.add_tree(node, os.path.join(path, entry), entry)
        else:
            node = Node(name, True)
            self.name_entry(name)
            parent.children.append(node)
            node.offset = self.input_offset
            with open(path, 'rb') as f:
                while True:
                    chunk = f.read(1024 * 1024)
                    if not chunk:
                        break
                    self.append(chunk)
                    node.size += len(chunk)

    def finalize(self):
        if self.block:
            self.append(bytes(BLOCK_SIZE - len(self.block)))
        sections = {}
        sections['data'] = (0, self.written)
        while self.written % 8:
            self.output(b'\0')

        # offset records: a 64-bit base offset, 16 16-bit sizes (all big endian)
        start = self.written
        for base, sizes in self.records:
            self.output(struct.pack('>Q16H', base, *sizes))
        sections['records'] = (start, self.written - start)

        # name table: a length byte (or two, MSB set, for 128+ bytes) + name
        start = self.written
        name_offsets = []
        pos = 0
        for name in self.names:
            raw = name.encode('utf-8')[:0x7FFF]
            name_offsets.append(pos)
            if len(raw) >= 0x80:
                hdr = bytes(((len(raw) & 0x7F) | 0x80, len(raw) >> 7))
            else:
                hdr = bytes((len(raw),))
            self.output(hdr + raw)
            pos += len(hdr) + len(raw)
        sections['names'] = (start, self.written - start)

        # file tree, breadth first: the root at index 0, the entries of each
        # directory contiguous and sorted
        queue = [self.root]
        index = 1
        i = 0
        while i < len(queue):
            node = queue[i]
            i += 1
            if node.is_file:
                continue
            node.children.sort(key=lambda n: sort_key(n.name))
            node.start_index = index
            index += len(node.children)
            queue.extend(node.children)
        start = self.written
        for node in queue:
            if node is self.root:
                name_field = ROOT_NAME_OFFSET
            else:
                name_field = name_offsets[self.name_index[node.name]]
            if node.is_file:
                name_field |= 0x80000000
                self.output(struct.pack('>IIII', name_field, node.offset & 0xFFFFFFFF, node.size & 0xFFFFFFFF,
                                        ((node.size >> 16) & 0xFFFF0000) | ((node.offset >> 32) & 0xFFFF)))
            else:
                self.output(struct.pack('>IIII', name_field, node.start_index, len(node.children), 0))
        sections['tree'] = (start, self.written - start)
        sections['metadir'] = (self.written, 0)
        sections['meta'] = (self.written, 0)

        def footer(digest):
            f = b''
            for key in ('data', 'records', 'names', 'tree', 'metadir', 'meta'):
                f += struct.pack('>QQ', *sections[key])
            f += digest
            f += struct.pack('>QII', self.written + 16 * 6 + 32 + 16, FOOTER_VERSION1, FOOTER_MAGIC)
            return f

        # the hash covers everything, the footer included with a zero hash
        self.sha.update(footer(bytes(32)))
        digest = self.sha.digest()
        self.out.write(footer(digest))


def main(argv):
    out = None
    dirs = []
    i = 0
    while i < len(argv):
        if argv[i] == '-o' and i + 1 < len(argv):
            out = argv[i + 1]
            i += 1
        else:
            dirs.append(argv[i])
        i += 1
    if not out or not dirs:
        sys.stderr.write('usage: mkwua.py -o out.wua dir [dir ...]\n')
        return 1
    tmp = out + '.tmp'
    with open(tmp, 'wb') as f:
        w = Writer(f)
        for d in dirs:
            w.add_tree(w.root, d, os.path.basename(os.path.normpath(d)))
        w.finalize()
    os.replace(tmp, out)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
