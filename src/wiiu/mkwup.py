#!/usr/bin/env python3
# Packs a Wii U title directory (code/, content/, meta/, as made by the
# GNUMakefile) into an installable title package (WUP / NUS format):
#
#   title.tmd        title metadata: the contents, their sizes and hashes
#   title.tik        ticket: the title key, encrypted with the common key
#   title.cert       certificate chain (only when -cert gives one)
#   00000000.app     content 0: the FST (the file table), unhashed
#   00000001.app     content 1: code/ (unhashed)
#   00000002.app     content 2: meta/ (hashed) + 00000002.h3
#   00000003.app     content 3: content/ (hashed) + 00000003.h3
#
# Every content is AES-128-CBC encrypted with the title key. An unhashed
# content is one CBC stream (IV: the content index, big endian, then
# zeros), padded to 0x8000 bytes, and the TMD keeps its SHA-1. A hashed
# content is cut into 0x10000 byte blocks: 0x400 bytes of hashes (the H0,
# H1 and H2 SHA-1 hashes of its groups, encrypted with a zero IV) and 0xFC00
# bytes of data (encrypted with the first 16 bytes of its own H0 as IV);
# the .h3 file holds one H3 hash per 4096 blocks and the TMD keeps the SHA-1
# of the .h3 file. This is the layout Cemu's FST reader
# (src/Cafe/Filesystem/FST/FST.cpp) and the Wii U's installer expect.
#
# The TMD and the ticket are not signed (a fake, all-zero signature): the
# console installs such a package only with signature patches (an Aroma /
# Tiramisu environment and an installer like WUP Installer GX2). The title
# key is encrypted with the Wii U common key, which this script does not
# carry: give it with -k <32 hex digits>, or -otp <otp.bin> (a dump of the
# console's OTP, where the common key is at 0xE0).
#
# Usage: mkwup.py -o outdir -t titleid -v version -g groupid
#                 (-k commonkey | -otp otp.bin) [-titlekey key] [-cert title.cert]
#                 titledir
#
# Only the standard library is used; AES comes from OpenSSL's libcrypto
# (ctypes), or the openssl command when the library cannot be loaded.

import ctypes
import ctypes.util
import hashlib
import os
import shutil
import struct
import subprocess
import sys

SECTOR = 0x8000
BLOCK = 0x10000
BLOCK_HASHES = 0x400
BLOCK_DATA = 0xFC00
OFFSET_FACTOR = 0x20
OS_VERSION = 0x000500101000400A

TYPE_ENCRYPTED = 0x0001
TYPE_HASHED = 0x0002
TYPE_SHA1 = 0x2000


# --- AES-128-CBC without padding -------------------------------------------

class _LibCrypto:
    def __init__(self):
        names = []
        for d in ('/usr/local/lib', '/opt/homebrew/lib', '/opt/homebrew/opt/openssl@3/lib'):
            for n in ('libcrypto.dylib', 'libcrypto.so', 'libcrypto.so.3'):
                names.append(os.path.join(d, n))
        found = ctypes.util.find_library('crypto')
        if found:
            names.append(found)
        self.lib = None
        for n in names:
            try:
                lib = ctypes.CDLL(n)
                lib.EVP_aes_128_cbc.restype = ctypes.c_void_p
                lib.EVP_CIPHER_CTX_new.restype = ctypes.c_void_p
                lib.EVP_CIPHER_CTX_free.argtypes = [ctypes.c_void_p]
                lib.EVP_EncryptInit_ex.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p,
                                                   ctypes.c_char_p, ctypes.c_char_p]
                lib.EVP_DecryptInit_ex.argtypes = lib.EVP_EncryptInit_ex.argtypes
                lib.EVP_CIPHER_CTX_set_padding.argtypes = [ctypes.c_void_p, ctypes.c_int]
                lib.EVP_EncryptUpdate.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_int),
                                                  ctypes.c_char_p, ctypes.c_int]
                lib.EVP_DecryptUpdate.argtypes = lib.EVP_EncryptUpdate.argtypes
                lib.EVP_aes_128_cbc()
                self.lib = lib
                break
            except (OSError, AttributeError):
                continue

    def crypt(self, data, key, iv, encrypt):
        lib = self.lib
        ctx = lib.EVP_CIPHER_CTX_new()
        try:
            init = lib.EVP_EncryptInit_ex if encrypt else lib.EVP_DecryptInit_ex
            update = lib.EVP_EncryptUpdate if encrypt else lib.EVP_DecryptUpdate
            if init(ctx, lib.EVP_aes_128_cbc(), None, key, iv) != 1:
                raise RuntimeError('EVP init failed')
            lib.EVP_CIPHER_CTX_set_padding(ctx, 0)
            out = ctypes.create_string_buffer(len(data) + 16)
            n = ctypes.c_int(0)
            if update(ctx, out, ctypes.byref(n), data, len(data)) != 1:
                raise RuntimeError('EVP update failed')
            return out.raw[:n.value]
        finally:
            lib.EVP_CIPHER_CTX_free(ctx)


_libcrypto = _LibCrypto()


def aes_cbc(data, key, iv, encrypt=True):
    assert len(data) % 16 == 0
    if not data:
        return b''
    if _libcrypto.lib is not None:
        return _libcrypto.crypt(bytes(data), key, iv, encrypt)
    args = ['openssl', 'enc', '-aes-128-cbc', '-nopad', '-K', key.hex(), '-iv', iv.hex()]
    if not encrypt:
        args.append('-d')
    return subprocess.run(args, input=bytes(data), stdout=subprocess.PIPE, check=True).stdout


def sha1(data):
    return hashlib.sha1(data).digest()


def pad(data, size):
    return data + bytes((-len(data)) % size)


# --- contents -------------------------------------------------------------

class Content:
    def __init__(self, index, hashed):
        self.index = index
        self.hashed = hashed
        self.data = bytearray()   # the files, OFFSET_FACTOR aligned

    def add(self, payload):
        offset = len(self.data)
        self.data += payload
        self.data += bytes((-len(self.data)) % OFFSET_FACTOR)
        return offset // OFFSET_FACTOR

    def type(self):
        return TYPE_SHA1 | TYPE_ENCRYPTED | (TYPE_HASHED if self.hashed else 0)


def build_raw(content, key):
    plain = pad(bytes(content.data), SECTOR)
    iv = struct.pack('>H', content.index) + bytes(14)
    return aes_cbc(plain, key, iv), sha1(plain), len(plain), None


def build_hashed(content, key):
    data = bytes(content.data)
    nblocks = max(1, (len(data) + BLOCK_DATA - 1) // BLOCK_DATA)
    data = data + bytes(nblocks * BLOCK_DATA - len(data))
    chunks = [data[i * BLOCK_DATA:(i + 1) * BLOCK_DATA] for i in range(nblocks)]
    h0 = [sha1(c) for c in chunks]
    ngroups = (nblocks + 4095) // 4096
    zero = bytes(20)
    h0 += [zero] * (ngroups * 4096 - nblocks)
    h1 = [sha1(b''.join(h0[i * 16:(i + 1) * 16])) for i in range(len(h0) // 16)]
    h2 = [sha1(b''.join(h1[i * 16:(i + 1) * 16])) for i in range(len(h1) // 16)]
    h3 = [sha1(b''.join(h2[i * 16:(i + 1) * 16])) for i in range(len(h2) // 16)]
    out = bytearray()
    for b in range(nblocks):
        g16 = b // 16 * 16
        g256 = b // 256 * 16
        g4096 = b // 4096 * 16
        hashes = b''.join(h0[g16:g16 + 16]) + b''.join(h1[g256:g256 + 16]) + b''.join(h2[g4096:g4096 + 16])
        hashes += bytes(BLOCK_HASHES - len(hashes))
        out += aes_cbc(hashes, key, bytes(16))
        out += aes_cbc(chunks[b], key, h0[b][:16])
    h3data = b''.join(h3)
    return bytes(out), sha1(h3data), len(out), h3data


# --- FST ------------------------------------------------------------------

class Entry:
    def __init__(self, name, is_dir, content=None, offset=0, size=0):
        self.name = name
        self.is_dir = is_dir
        self.content = content
        self.offset = offset
        self.size = size
        self.children = []


def name_key(name):
    return name.lower()


def add_tree(entry, path, content, contents):
    for name in sorted(os.listdir(path), key=name_key):
        if name.startswith('.'):
            continue
        p = os.path.join(path, name)
        if os.path.isdir(p):
            d = Entry(name, True)
            add_tree(d, p, content, contents)
            entry.children.append(d)
        else:
            with open(p, 'rb') as f:
                payload = f.read()
            c = contents[content]
            entry.children.append(Entry(name, False, content, c.add(payload), len(payload)))


def build_fst(root, clusters, title_id, group_id):
    # flatten depth first: a directory's offset is its parent's index, its
    # size the index after its last descendant
    flat = []

    def walk(e, parent):
        index = len(flat)
        flat.append([e, parent, 0])
        for c in e.children:
            walk(c, index)
        flat[index][2] = len(flat)

    walk(root, 0)
    names = bytearray(b'\0')
    name_offsets = []
    for e, _, _ in flat:
        if e is root:
            name_offsets.append(0)
        else:
            name_offsets.append(len(names))
            names += e.name.encode('utf-8') + b'\0'

    out = bytearray(struct.pack('>4sIIBBBBIIII', b'FST\0', OFFSET_FACTOR, len(clusters), 0, 0, 0, 0, 0, 0, 0, 0))
    sector = 0
    for size, hash_mode in clusters:
        nsect = (size + SECTOR - 1) // SECTOR
        out += struct.pack('>IIQIB11x', sector, nsect, title_id, group_id, hash_mode)
        sector += nsect
    for i, (e, parent, end) in enumerate(flat):
        if e.is_dir:
            out += struct.pack('>IIIHH', 0x01000000 | name_offsets[i], parent, end, 0, 0)
        else:
            out += struct.pack('>IIIHH', name_offsets[i], e.offset, e.size, 0, e.content)
    out += names
    return bytes(out)


# --- TMD and ticket -------------------------------------------------------

def build_tmd(title_id, version, group_id, records):
    rec = b''
    for content_id, index, ctype, size, digest in records:
        rec += struct.pack('>IHHQ', content_id, index, ctype, size) + digest + bytes(32 - len(digest))
    info = struct.pack('>HH', 0, len(records)) + hashlib.sha256(rec).digest()
    info += bytes(36 * 63)
    hdr = bytearray(0x204)
    struct.pack_into('>I', hdr, 0, 0x00010004)                       # RSA-2048 SHA-256 (fake)
    hdr[0x140:0x140 + 26] = b'Root-CA00000003-CP0000000b'
    hdr[0x180] = 1                                                    # TMD version
    struct.pack_into('>Q', hdr, 0x184, OS_VERSION)
    struct.pack_into('>Q', hdr, 0x18C, title_id)
    struct.pack_into('>I', hdr, 0x194, 0x00000100)                   # title type
    struct.pack_into('>H', hdr, 0x198, group_id & 0xFFFF)
    struct.pack_into('>H', hdr, 0x1DC, version)
    struct.pack_into('>H', hdr, 0x1DE, len(records))
    struct.pack_into('>H', hdr, 0x1E0, 0)                            # boot content
    hdr[0x1E4:0x204] = hashlib.sha256(info).digest()
    return bytes(hdr) + info + rec


def build_ticket(title_id, version, enc_title_key):
    t = bytearray(0x2A4)
    struct.pack_into('>I', t, 0, 0x00010004)
    t[0x140:0x140 + 26] = b'Root-CA00000003-XS0000000c'
    t[0x1BC] = 1                                                      # ticket format version
    t[0x1BF:0x1CF] = enc_title_key
    struct.pack_into('>Q', t, 0x1D0, 0x0005000000000000 | (title_id & 0xFFFFFFFF))   # ticket id
    struct.pack_into('>Q', t, 0x1DC, title_id)
    struct.pack_into('>H', t, 0x1E6, version)
    t[0x222:0x222 + 0x40] = b'\xff' * 0x40                            # content access mask
    # v1 extension: one content rights section, every content allowed
    ext = struct.pack('>HHIIHHI', 1, 0x14, 0xAC, 0x14, 1, 0x14, 0)
    ext += struct.pack('>IIIIHH', 0x28, 1, 0x84, 0x84, 3, 0)
    ext += struct.pack('>I', 0) + b'\xff' * 0x80
    return bytes(t) + ext


# --- main -----------------------------------------------------------------

def main(argv):
    opts = {}
    args = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a in ('-o', '-t', '-v', '-g', '-k', '-otp', '-titlekey', '-cert') and i + 1 < len(argv):
            opts[a] = argv[i + 1]
            i += 2
        else:
            args.append(a)
            i += 1
    if len(args) != 1 or '-o' not in opts or '-t' not in opts:
        sys.stderr.write('usage: mkwup.py -o outdir -t titleid -v version -g groupid (-k commonkey | -otp otp.bin) '
                         '[-titlekey key] [-cert title.cert] titledir\n')
        return 1
    titledir = args[0]
    title_id = int(opts['-t'], 16)
    version = int(opts.get('-v', '0'), 0)
    group_id = int(opts.get('-g', '0'), 16)

    if '-k' in opts:
        common_key = bytes.fromhex(opts['-k'].strip())
    elif '-otp' in opts:
        with open(opts['-otp'], 'rb') as f:
            otp = f.read()
        common_key = otp[0xE0:0xF0]
    else:
        sys.stderr.write('mkwup.py: the Wii U common key is needed (-k <hex> or -otp <otp.bin>)\n')
        return 1
    if len(common_key) != 16:
        sys.stderr.write('mkwup.py: the common key must be 16 bytes\n')
        return 1
    # the title key: given, or derived from the title id (any key works, it
    # only has to be the same in the ticket and the contents)
    if '-titlekey' in opts:
        title_key = bytes.fromhex(opts['-titlekey'])
    else:
        title_key = hashlib.sha256(b'PumpkinOS title key ' + struct.pack('>Q', title_id)).digest()[:16]
    enc_title_key = aes_cbc(title_key, common_key, struct.pack('>Q', title_id) + bytes(8))

    # content 0 is the FST, then code/ (unhashed), meta/ and content/ (hashed)
    contents = {1: Content(1, False), 2: Content(2, True), 3: Content(3, True)}
    root = Entry('', True)
    for name, index in (('code', 1), ('content', 3), ('meta', 2)):
        p = os.path.join(titledir, name)
        if os.path.isdir(p):
            d = Entry(name, True)
            add_tree(d, p, index, contents)
            root.children.append(d)
    if not contents[1].data:
        sys.stderr.write(f'mkwup.py: {titledir}/code is empty\n')
        return 1
    used = [c for _, c in sorted(contents.items()) if c.data]
    # (the content indexes must be consecutive: renumber when one is empty)
    for n, c in enumerate(used, 1):
        if c.index != n:
            for e in iter_files(root):
                if e.content == c.index:
                    e.content = n
            c.index = n

    built = []
    for c in used:
        built.append((c,) + (build_hashed(c, title_key) if c.hashed else build_raw(c, title_key)))

    clusters = [(0, 1)] + [(size, 2 if c.hashed else 1) for c, _, _, size, _ in built]
    fst = build_fst(root, clusters, title_id, group_id)
    clusters[0] = (len(pad(fst, SECTOR)), 1)
    fst = build_fst(root, clusters, title_id, group_id)
    fst_content = Content(0, False)
    fst_content.data = bytearray(fst)
    fst_built = (fst_content,) + build_raw(fst_content, title_key)

    if os.path.isdir(opts['-o']):
        shutil.rmtree(opts['-o'])
    os.makedirs(opts['-o'])
    records = []
    for c, enc, digest, size, h3 in [fst_built] + built:
        with open(os.path.join(opts['-o'], f'{c.index:08x}.app'), 'wb') as f:
            f.write(enc)
        if h3 is not None:
            with open(os.path.join(opts['-o'], f'{c.index:08x}.h3'), 'wb') as f:
                f.write(h3)
        records.append((c.index, c.index, c.type(), size, digest))
    with open(os.path.join(opts['-o'], 'title.tmd'), 'wb') as f:
        f.write(build_tmd(title_id, version, group_id, records))
    with open(os.path.join(opts['-o'], 'title.tik'), 'wb') as f:
        f.write(build_ticket(title_id, version, enc_title_key))
    if '-cert' in opts:
        shutil.copyfile(opts['-cert'], os.path.join(opts['-o'], 'title.cert'))
    return 0


def iter_files(entry):
    for c in entry.children:
        if c.is_dir:
            yield from iter_files(c)
        else:
            yield c


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
