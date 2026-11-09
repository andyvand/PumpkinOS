#!/usr/bin/env python3
# Lays out a Wii disc directory, as Wiimms ISO Tools (wit) composes it into a
# fake signed disc image (wit copy <dir> PumpkinOS.wbfs, see GNUMakefile):
#
#   <dir>/sys/boot.bin       disc header: ID6, title, Wii magic (wit fills in
#                            the DOL and FST offsets)
#   <dir>/sys/bi2.bin        disc header information
#   <dir>/sys/apploader.img  32 byte header + the apploader (apploader/)
#   <dir>/sys/main.dol       the program
#   <dir>/files/             the disc file system (README.md)
#
# usage: mkdisc.py -o <dir> -id <ID6> -title <title> -apploader <bin>
#                  -dol <dol> [file ...]

import os
import shutil
import struct
import sys
import time

APPLOADER_ADDR = 0x81200000
WII_MAGIC = 0x5D1C9EA3


def usage():
    sys.stderr.write("usage: mkdisc.py -o <dir> -id <ID6> -title <title> -apploader <bin> -dol <dol> [file ...]\n")
    sys.exit(1)


def main(argv):
    out = disc_id = title = apploader = dol = None
    files = []
    i = 1
    while i < len(argv):
        a = argv[i]
        if a in ("-o", "-id", "-title", "-apploader", "-dol") and i + 1 < len(argv):
            v = argv[i + 1]
            i += 2
            if a == "-o": out = v
            elif a == "-id": disc_id = v
            elif a == "-title": title = v
            elif a == "-apploader": apploader = v
            else: dol = v
        elif a.startswith("-"):
            usage()
        else:
            files.append(a)
            i += 1
    if not (out and disc_id and title and apploader and dol) or len(disc_id) != 6:
        usage()

    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(os.path.join(out, "sys"))
    os.makedirs(os.path.join(out, "files"))

    # boot.bin (0x440 bytes)
    boot = bytearray(0x440)
    boot[0:6] = disc_id.encode("ascii")
    struct.pack_into(">I", boot, 0x18, WII_MAGIC)
    t = title.encode("ascii")[:0x3df]
    boot[0x20:0x20 + len(t)] = t
    with open(os.path.join(out, "sys", "boot.bin"), "wb") as f:
        f.write(boot)

    # bi2.bin (0x2000 bytes): simulated memory size 24MB, country code USA
    # (wit --region sets the region the console checks)
    bi2 = bytearray(0x2000)
    struct.pack_into(">I", bi2, 0x04, 0x01800000)
    struct.pack_into(">I", bi2, 0x18, 1)
    with open(os.path.join(out, "sys", "bi2.bin"), "wb") as f:
        f.write(bi2)

    # apploader.img: date, entry point, size, trailer size, then the code
    with open(apploader, "rb") as f:
        code = f.read()
    code += b"\0" * ((32 - len(code) % 32) % 32)
    hdr = bytearray(0x20)
    date = time.strftime("%Y/%m/%d").encode("ascii")
    hdr[0:len(date)] = date
    struct.pack_into(">III", hdr, 0x10, APPLOADER_ADDR, len(code), 0)
    with open(os.path.join(out, "sys", "apploader.img"), "wb") as f:
        f.write(hdr + code)

    shutil.copyfile(dol, os.path.join(out, "sys", "main.dol"))
    for name in files:
        shutil.copyfile(name, os.path.join(out, "files", os.path.basename(name)))


if __name__ == "__main__":
    main(sys.argv)
