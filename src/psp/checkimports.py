#!/usr/bin/env python3
# Checks the import tables of a linked PSP ELF: every module listed in
# .lib.stub must own a contiguous run of entries in .rodata.sceNid, and the
# runs must not overlap. GNU ld pulls the pspsdk import stubs (one archive
# member per function) in the order it resolves them; a stub library that is
# scanned more than once contributes its members in several batches, which
# scatters a module's NIDs between other modules' and makes the firmware
# bind the wrong functions (psp-fixup-imports does not detect this).
# Usage: checkimports.py <psp-objdump> <file.elf>
import re
import struct
import subprocess
import sys


def section(objdump, elf, name):
    out = subprocess.run([objdump, "-s", "-j", name, elf], capture_output=True, text=True).stdout
    data = {}
    for line in out.splitlines():
        m = re.match(r"^ ([0-9a-f]+) ((?:[0-9a-f]{2,8} ?){1,4})", line)
        if m:
            data[int(m.group(1), 16)] = bytes.fromhex(m.group(2).replace(" ", ""))
    if not data:
        sys.exit("checkimports: section %s not found in %s" % (name, elf))
    return min(data), b"".join(data[a] for a in sorted(data))


def main():
    objdump, elf = sys.argv[1], sys.argv[2]
    sb, stub = section(objdump, elf, ".lib.stub")
    rb, res = section(objdump, elf, ".rodata.sceResident")
    nb, nid = section(objdump, elf, ".rodata.sceNid")
    total = len(nid) // 4
    modules = []
    for i in range(0, len(stub), 20):
        namep, flags, stublen, idtable, text = struct.unpack_from("<IIIII", stub, i)
        o = namep - rb
        name = res[o:res.index(b"\0", o)].decode()
        start = (idtable - nb) // 4
        modules.append((start, start + (stublen >> 16), name))
    modules.sort()
    bad = []
    for n, (start, end, name) in enumerate(modules):
        if n + 1 < len(modules) and end > modules[n + 1][0]:
            bad.append("%s (NIDs %d..%d) overlaps %s (from %d)" % (name, start, end - 1, modules[n + 1][2], modules[n + 1][0]))
        if end > total:
            bad.append("%s (NIDs %d..%d) runs past the table (%d entries)" % (name, start, end - 1, total))
    if bad:
        print("checkimports: broken import table in %s:" % elf)
        for b in bad:
            print("  " + b)
        print("  (a sce stub library is probably linked twice, see PSPLIBS in GNUMakefile)")
        sys.exit(1)
    print("Import table ok (%d modules, %d functions)" % (len(modules), total))


main()
