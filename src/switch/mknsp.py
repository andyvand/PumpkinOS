#!/usr/bin/env python3
# Packs PumpkinOS as an installable Nintendo Switch title (NSP):
#
#   <id>.nca       Program NCA: section 0 the ExeFS (PFS0 with the main NSO
#                  and main.npdm), section 1 the RomFS (the same data the NRO
#                  embeds)
#   <id>.nca       Control NCA: a RomFS with control.nacp and
#                  icon_AmericanEnglish.dat (the 256x256 JPEG icon)
#   <id>.cnmt.nca  Meta NCA: a PFS0 with Application_<titleid>.cnmt, which
#                  lists the two other NCAs with their sizes and SHA-256
#
# With -aoc <index> (1-2000) an add-on content (DLC) NSP is made instead,
# title ID = application title ID + 0x1000 + index:
#
#   <id>.nca       PublicData NCA: a RomFS (-romfs romfs.bin)
#   <id>.cnmt.nca  Meta NCA: AddOnContent_<titleid>.cnmt (type 0x82, with the
#                  application title ID in its extended header)
#
# The NSP itself is a PFS0 of the three (two for a DLC) NCAs. Each NCA name is the first 16
# bytes of the NCA's SHA-256, in hex.
#
# NCA format (NCA3), as read by hactool, LibHac (Ryujinx) and the console:
# a 0xC00 byte header encrypted with AES-128-XTS (header_key, 0x200 byte
# sectors, big endian sector number as tweak), holding up to four section
# headers; every section is encrypted with AES-128-CTR (counter: the
# section's 8 byte upper IV, then the absolute NCA offset / 16, big endian)
# with the key stored in slot 2 of the key area, which is AES-128-ECB
# encrypted with key_area_key_application_<generation>. PFS0 sections are
# covered by a one level SHA-256 table (0x10000 byte blocks, the last block
# hashed as it is), RomFS sections by a six level IVFC tree (0x4000 byte
# blocks, the last block zero padded).
#
# Signatures: an NCA header carries two RSA-2048-PSS (SHA-256) signatures
# of its bytes 0x200-0x3FF. Signature 1 is checked with a fixed Nintendo key
# and the ACID of main.npdm is signed with another fixed Nintendo key; their
# private halves are not public, so both stay zero. Signature 2 of the
# Program NCA is checked with the RSA modulus stored in the ACID itself: with
# -sign key.pem (an RSA-2048 key, exponent 65537) that modulus is written
# into main.npdm and signature 2 is made with the key. The console still
# installs and runs the NSP only with signature patches (Atmosphere with
# sigpatches for the fixed-key checks + an installer like DBI, Tinfoil or
# Goldleaf), and there is no rights ID (no ticket). Ryujinx loads the NSP
# directly.
#
# The keys come from a prod.keys file (header_key and
# key_area_key_application_XX), which this script does not carry.
#
# Usage: mknsp.py -k prod.keys -t titleid -o out.nsp
#                 -exefs exefsdir -romfs romfs.bin -nacp control.nacp
#                 -icon icon.jpg [-v version] [-keygen n] [-sdk version]
#                 [-sign key.pem]
#        mknsp.py -k prod.keys -t titleid -o out.nsp -aoc index -romfs romfs.bin
#                 [-v version] [-keygen n] [-sdk version]
#
# romfs.bin is a RomFS image (build_romfs from devkitPro). Only the standard
# library is used; AES comes from OpenSSL's libcrypto (ctypes), RSA from
# the openssl command.

import ctypes
import ctypes.util
import hashlib
import os
import struct
import subprocess
import sys
import tempfile

MEDIA = 0x200
HEADER_SIZE = 0xC00
PFS_BLOCK = 0x10000
IVFC_BLOCK_LOG2 = 14
IVFC_BLOCK = 1 << IVFC_BLOCK_LOG2
IVFC_LEVELS = 6

NCA_PROGRAM = 0
NCA_META = 1
NCA_CONTROL = 2
NCA_PUBLIC_DATA = 5

CNMT_PROGRAM = 1
CNMT_CONTROL = 3
CNMT_DATA = 2
CNMT_APPLICATION = 0x80
CNMT_ADD_ON_CONTENT = 0x82

FS_ROMFS = 0
FS_PFS0 = 1
HASH_SHA256 = 2
HASH_IVFC = 3
CRYPT_CTR = 3


# --- AES (OpenSSL libcrypto through ctypes) ---------------------------------

class LibCrypto:
    def __init__(self):
        names = []
        for d in ('/usr/local/lib', '/opt/homebrew/lib', '/opt/homebrew/opt/openssl@3/lib'):
            for n in ('libcrypto.dylib', 'libcrypto.so', 'libcrypto.so.3'):
                names.append(os.path.join(d, n))
        found = ctypes.util.find_library('crypto')
        if found:
            names.append(found)
        for n in ('libcrypto.so.3', 'libcrypto.so'):
            names.append(n)
        self.lib = None
        for n in names:
            try:
                lib = ctypes.CDLL(n)
                for c in ('EVP_aes_128_ecb', 'EVP_aes_128_ctr', 'EVP_aes_128_xts'):
                    getattr(lib, c).restype = ctypes.c_void_p
                lib.EVP_CIPHER_CTX_new.restype = ctypes.c_void_p
                lib.EVP_CIPHER_CTX_free.argtypes = [ctypes.c_void_p]
                lib.EVP_CipherInit_ex.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p,
                                                  ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
                lib.EVP_CIPHER_CTX_set_padding.argtypes = [ctypes.c_void_p, ctypes.c_int]
                lib.EVP_CipherUpdate.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_int),
                                                 ctypes.c_char_p, ctypes.c_int]
                lib.EVP_aes_128_xts()
                self.lib = lib
                break
            except (OSError, AttributeError):
                continue
        if self.lib is None:
            sys.exit('mknsp: OpenSSL libcrypto not found')

    def crypt(self, cipher, key, iv, data, encrypt=True):
        lib = self.lib
        ctx = lib.EVP_CIPHER_CTX_new()
        try:
            if lib.EVP_CipherInit_ex(ctx, getattr(lib, cipher)(), None, key, iv, 1 if encrypt else 0) != 1:
                raise RuntimeError('EVP init failed')
            lib.EVP_CIPHER_CTX_set_padding(ctx, 0)
            out = ctypes.create_string_buffer(len(data) + 16)
            n = ctypes.c_int(0)
            if lib.EVP_CipherUpdate(ctx, out, ctypes.byref(n), bytes(data), len(data)) != 1:
                raise RuntimeError('EVP update failed')
            return out.raw[:n.value]
        finally:
            lib.EVP_CIPHER_CTX_free(ctx)


crypto = LibCrypto()


def aes_ecb(key, data, encrypt=True):
    return crypto.crypt('EVP_aes_128_ecb', key, None, data, encrypt)


def aes_ctr(key, ctr, data):
    return crypto.crypt('EVP_aes_128_ctr', key, ctr, data)


def aes_xts_nintendo(key, data, encrypt=True):
    # one XTS unit per 0x200 byte sector, the tweak is the sector number
    # as a big endian 128-bit value (standard XTS uses little endian)
    out = bytearray()
    for i in range(0, len(data), MEDIA):
        tweak = (i // MEDIA).to_bytes(16, 'big')
        out += crypto.crypt('EVP_aes_128_xts', key, tweak, data[i:i + MEDIA], encrypt)
    return bytes(out)


# --- RSA-2048-PSS (openssl command) ------------------------------------------

class Signer:
    def __init__(self, path):
        self.path = path
        out = subprocess.run(['openssl', 'rsa', '-in', path, '-noout', '-text'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if out.returncode != 0:
            sys.exit('mknsp: cannot read the RSA key %s: %s' % (path, out.stderr.strip()))
        if 'publicExponent: 65537' not in out.stdout or '2048 bit' not in out.stdout:
            sys.exit('mknsp: %s must be an RSA-2048 key with exponent 65537' % path)
        out = subprocess.run(['openssl', 'rsa', '-in', path, '-noout', '-modulus'],
                             stdout=subprocess.PIPE, check=True, text=True)
        self.modulus = bytes.fromhex(out.stdout.strip().split('=', 1)[1])
        assert len(self.modulus) == 0x100

    def sign(self, data):
        with tempfile.TemporaryDirectory() as tmp:
            src = os.path.join(tmp, 'data')
            with open(src, 'wb') as f:
                f.write(data)
            sig = subprocess.run(['openssl', 'dgst', '-sha256', '-sign', self.path,
                                  '-sigopt', 'rsa_padding_mode:pss', '-sigopt', 'rsa_pss_saltlen:32',
                                  '-sigopt', 'rsa_mgf1_md:sha256', src],
                                 stdout=subprocess.PIPE, check=True).stdout
        assert len(sig) == 0x100
        return sig


def npdm_set_acid_key(npdm, modulus):
    npdm = bytearray(npdm)
    if npdm[:4] != b'META':
        sys.exit('mknsp: main.npdm is not an NPDM')
    acid, acid_size = struct.unpack_from('<II', npdm, 0x78)
    if acid_size < 0x210 or npdm[acid + 0x200:acid + 0x204] != b'ACID':
        sys.exit('mknsp: main.npdm has no ACID')
    npdm[acid + 0x100:acid + 0x200] = modulus
    return bytes(npdm)


# --- helpers ----------------------------------------------------------------

def align(n, a):
    return (n + a - 1) // a * a


def pad(data, a):
    return bytes(data) + bytes(align(len(data), a) - len(data))


def sha256(data):
    return hashlib.sha256(data).digest()


def read_keys(path):
    keys = {}
    with open(path) as f:
        for line in f:
            line = line.split('#')[0].strip()
            if '=' in line:
                k, v = line.split('=', 1)
                try:
                    keys[k.strip().lower()] = bytes.fromhex(v.strip())
                except ValueError:
                    pass
    return keys


def pfs0(files):
    # files: list of (name, data). The header (with the string table) is
    # padded to 0x20 bytes, the file data follows it back to back.
    strings = bytearray()
    entries = bytearray()
    offset = 0
    for name, data in files:
        entries += struct.pack('<QQII', offset, len(data), len(strings), 0)
        strings += name.encode() + b'\0'
        offset += len(data)
    header_size = 0x10 + len(entries) + len(strings)
    strings += bytes(align(header_size, 0x20) - header_size)
    out = bytearray(b'PFS0' + struct.pack('<III', len(files), len(strings), 0))
    out += entries + strings
    for _, data in files:
        out += data
    return bytes(out)


# --- NCA sections -----------------------------------------------------------

class Section:
    def __init__(self, fs_type, hash_type, data, hash_info):
        self.fs_type = fs_type
        self.hash_type = hash_type
        self.data = data            # plaintext section (hash levels + data)
        self.hash_info = hash_info  # 0xF8 bytes at fs header 0x8


def pfs0_section(pfs):
    # level 0: SHA-256 of every 0x10000 byte block of the PFS0
    table = b''.join(sha256(pfs[i:i + PFS_BLOCK]) for i in range(0, len(pfs), PFS_BLOCK))
    pfs_offset = align(len(table), MEDIA)
    data = pad(table, MEDIA) + pfs
    info = sha256(table) + struct.pack('<II', PFS_BLOCK, 2)
    info += struct.pack('<QQ', 0, len(table)) + struct.pack('<QQ', pfs_offset, len(pfs))
    return Section(FS_PFS0, HASH_SHA256, data, info.ljust(0xF8, b'\0'))


def ivfc_hashes(level):
    out = bytearray()
    for i in range(0, max(len(level), 1), IVFC_BLOCK):
        out += sha256(level[i:i + IVFC_BLOCK].ljust(IVFC_BLOCK, b'\0'))
    return bytes(out)


def romfs_section(romfs):
    # levels[5] is the RomFS, levels[i] the hashes of the blocks of
    # levels[i+1]; the master hash covers levels[0] (one block)
    levels = [romfs]
    for _ in range(IVFC_LEVELS - 1):
        levels.insert(0, ivfc_hashes(levels[0]))
    master = ivfc_hashes(levels[0])
    assert len(master) == 0x20
    data = bytearray()
    info = bytearray(b'IVFC' + struct.pack('<III', 0x20000, 0x20, IVFC_LEVELS + 1))
    for level in levels:
        offset = len(data)
        data += level
        info += struct.pack('<QQII', offset, len(level), IVFC_BLOCK_LOG2, 0)
        data += bytes(align(len(data), IVFC_BLOCK) - len(data))
    info += bytes(0x20)   # signature salt
    info += master
    return Section(FS_ROMFS, HASH_IVFC, bytes(data), bytes(info).ljust(0xF8, b'\0'))


def make_nca(keys, content_type, title_id, sections, keygen, sdk, signer=None):
    header_key = keys.get('header_key')
    if header_key is None or len(header_key) != 0x20:
        sys.exit('mknsp: header_key missing in the key file')
    master = max(keygen - 1, 0)
    kaek = keys.get('key_area_key_application_%02x' % master)
    if kaek is None:
        sys.exit('mknsp: key_area_key_application_%02x missing in the key file' % master)

    ctr_key = sha256(b'PumpkinOS NCA key' + struct.pack('<QB', title_id, content_type))[:16]
    key_area = bytes(0x20) + ctr_key + bytes(0x10)

    header = bytearray(HEADER_SIZE)
    body = bytearray()
    offset = HEADER_SIZE
    for i, s in enumerate(sections):
        plain = pad(s.data, MEDIA)
        fs = bytearray(MEDIA)
        struct.pack_into('<HBBBB', fs, 0, 2, s.fs_type, s.hash_type, CRYPT_CTR, 0)
        fs[0x8:0x8 + 0xF8] = s.hash_info
        upper = (i + 1) << 32           # secure value = section + 1, generation 0
        struct.pack_into('<Q', fs, 0x140, upper)
        ctr = upper.to_bytes(8, 'big') + (offset >> 4).to_bytes(8, 'big')
        body += aes_ctr(ctr_key, ctr, plain)
        struct.pack_into('<II', header, 0x240 + i * 0x10, offset // MEDIA, (offset + len(plain)) // MEDIA)
        header[0x280 + i * 0x20:0x2A0 + i * 0x20] = sha256(fs)
        header[0x400 + i * MEDIA:0x600 + i * MEDIA] = fs
        offset += len(plain)

    header[0x200:0x204] = b'NCA3'
    header[0x204] = 0                   # distribution: download
    header[0x205] = content_type
    header[0x206] = min(keygen, 2)      # key generation (old field)
    header[0x207] = 0                   # key area encryption key: application
    struct.pack_into('<QQIIBB', header, 0x208, offset, title_id, 0, sdk,
                     keygen if keygen > 2 else 0, 0)
    header[0x300:0x340] = aes_ecb(kaek, key_area)
    if signer is not None:
        header[0x100:0x200] = signer.sign(bytes(header[0x200:0x400]))  # signature 2 (ACID key)
    return aes_xts_nintendo(header_key, bytes(header)) + bytes(body)


# --- content meta -----------------------------------------------------------

def cnmt(title_id, version, contents, application_id=None):
    meta_type = CNMT_APPLICATION if application_id is None else CNMT_ADD_ON_CONTENT
    out = bytearray(struct.pack('<QIBBHHHB3xI4x', title_id, version, meta_type, 0,
                                0x10, len(contents), 0, 0, 0))
    if application_id is None:
        out += struct.pack('<QII', title_id | 0x800, 0, 0)  # patch ID, required system / app version
    else:
        out += struct.pack('<QII', application_id, 0, 0)    # application ID, required app version
    for ctype, nca in contents:
        h = sha256(nca)
        out += h + h[:16] + len(nca).to_bytes(6, 'little') + bytes([ctype, 0])
    out += bytes(0x20)   # digest
    return bytes(out)


def patch_nacp(nacp, title_id):
    nacp = bytearray(nacp)
    if len(nacp) != 0x4000:
        sys.exit('mknsp: control.nacp must be 0x4000 bytes')
    flags = struct.unpack_from('<I', nacp, 0x302C)[0]
    struct.pack_into('<I', nacp, 0x302C, flags | 1)      # supported language: AmericanEnglish
    struct.pack_into('<Q', nacp, 0x3038, title_id)       # presence group ID
    struct.pack_into('<Q', nacp, 0x3070, title_id + 0x1000)  # add-on content base ID
    struct.pack_into('<Q', nacp, 0x3078, title_id)       # save data owner ID
    if struct.unpack_from('<Q', nacp, 0x30B0)[0] == 0:
        struct.pack_into('<Q', nacp, 0x30B0, title_id)   # local communication ID
    return bytes(nacp)


def build_romfs(directory, tool):
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, 'romfs.bin')
        subprocess.run([tool, directory, out], check=True, stdout=subprocess.DEVNULL)
        with open(out, 'rb') as f:
            return f.read()


def write_nsp(path, ncas):
    files = [(sha256(nca)[:16].hex() + suffix, nca) for nca, suffix in ncas]
    with open(path, 'wb') as f:
        f.write(pfs0(files))
    for name, nca in files:
        print('  %s %d bytes' % (name, len(nca)))


def main():
    args = {'-v': '0', '-keygen': '0', '-sdk': '0x000C1100',
            '-build_romfs': os.path.join(os.environ.get('DEVKITPRO', '/opt/devkitpro'), 'tools/bin/build_romfs')}
    argv = sys.argv[1:]
    if len(argv) % 2:
        sys.exit(__doc__ or 'usage: see mknsp.py')
    for i in range(0, len(argv), 2):
        args[argv[i]] = argv[i + 1]
    required = ('-k', '-t', '-o', '-romfs') if '-aoc' in args else \
        ('-k', '-t', '-o', '-exefs', '-romfs', '-nacp', '-icon')
    for req in required:
        if req not in args:
            sys.exit('mknsp: %s missing (usage: mknsp.py -k prod.keys -t titleid -o out.nsp -exefs dir '
                     '-romfs romfs.bin -nacp control.nacp -icon icon.jpg [-v version] [-keygen n] '
                     '[-sign key.pem], or -k prod.keys -t titleid -o out.nsp -aoc index -romfs romfs.bin)' % req)

    keys = read_keys(args['-k'])
    title_id = int(args['-t'], 16)
    version = int(args['-v'], 0)
    keygen = int(args['-keygen'], 0)
    sdk = int(args['-sdk'], 16)
    if title_id & 0x1FFF:
        sys.exit('mknsp: an application title ID must end in 0x000 (bits 0-12 clear)')

    if '-aoc' in args:
        index = int(args['-aoc'], 0)
        if not 1 <= index <= 2000:
            sys.exit('mknsp: the add-on content index must be 1-2000')
        aoc_id = title_id + 0x1000 + index
        with open(args['-romfs'], 'rb') as f:
            romfs = f.read()
        data = make_nca(keys, NCA_PUBLIC_DATA, aoc_id, [romfs_section(romfs)], keygen, sdk)
        meta_name = 'AddOnContent_%016x.cnmt' % aoc_id
        meta_data = cnmt(aoc_id, version, [(CNMT_DATA, data)], title_id)
        meta = make_nca(keys, NCA_META, aoc_id, [pfs0_section(pfs0([(meta_name, meta_data)]))], keygen, sdk)
        write_nsp(args['-o'], ((data, '.nca'), (meta, '.cnmt.nca')))
        return

    signer = Signer(args['-sign']) if '-sign' in args else None
    exefs = []
    for name in ('main', 'main.npdm'):
        with open(os.path.join(args['-exefs'], name), 'rb') as f:
            data = f.read()
        if name == 'main.npdm' and signer is not None:
            data = npdm_set_acid_key(data, signer.modulus)
        exefs.append((name, data))
    with open(args['-romfs'], 'rb') as f:
        romfs = f.read()
    program = make_nca(keys, NCA_PROGRAM, title_id,
                       [pfs0_section(pfs0(exefs)), romfs_section(romfs)], keygen, sdk, signer)

    with open(args['-nacp'], 'rb') as f:
        nacp = patch_nacp(f.read(), title_id)
    with open(args['-icon'], 'rb') as f:
        icon = f.read()
    with tempfile.TemporaryDirectory() as tmp:
        with open(os.path.join(tmp, 'control.nacp'), 'wb') as f:
            f.write(nacp)
        with open(os.path.join(tmp, 'icon_AmericanEnglish.dat'), 'wb') as f:
            f.write(icon)
        control_romfs = build_romfs(tmp, args['-build_romfs'])
    control = make_nca(keys, NCA_CONTROL, title_id, [romfs_section(control_romfs)], keygen, sdk)

    meta_name = 'Application_%016x.cnmt' % title_id
    meta_data = cnmt(title_id, version, [(CNMT_PROGRAM, program), (CNMT_CONTROL, control)])
    meta = make_nca(keys, NCA_META, title_id, [pfs0_section(pfs0([(meta_name, meta_data)]))], keygen, sdk)

    write_nsp(args['-o'], ((program, '.nca'), (control, '.nca'), (meta, '.cnmt.nca')))


if __name__ == '__main__':
    main()
