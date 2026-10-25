#!/usr/bin/env python3
"""Did moving these functions leave their machine code alone?

A pure move (a function cut from one .c and pasted into another, text
unchanged) should give the same instructions. Comparing
`objdump -d --disassemble=<fn>` output directly does not work, because the
move shifts addresses:

  - every instruction address and every call/jump target;
  - rip-relative operands, e.g. `lea 0x1234(%rip),%rdi   # 5678 <.rodata+0x42>`.
    For string constants the comment names only a section offset, which
    changes whenever .rodata is laid out differently, even though the string
    is the same. (Comments that name a symbol, like `<py>` or `<CSWTCH.12>`,
    shift in the same way when the symbol number changes.)

This script strips addresses, and replaces each rip-relative operand with the
bytes it points at when they lie in .rodata (read up to the first NUL, which
is the string for string constants). It then compares the two listings
function by function.

Usage:
    python3 scripts/dis_compare.py OLD_BINARY NEW_BINARY fn [fn ...]
    python3 scripts/dis_compare.py --dump DIR BINARY fn [fn ...]

The first form prints `same` or `DIFFERENT` for each function and exits 1 if
any differ (or are missing). The second writes DIR/<fn>.txt, the normalized
listing, so the two can be read side by side with diff.

Build both binaries with the same flags (the main makefile). Before trusting
a `same`, make one deliberate change to a moved function and check that it
turns into `DIFFERENT`.
"""
import argparse
import os
import re
import subprocess
import sys


def sections(binary):
    """(name, vma, size, file offset) for each section."""
    out = subprocess.run(['objdump', '-h', binary], capture_output=True,
                         text=True, check=True).stdout
    secs = []
    for line in out.splitlines():
        p = line.split()
        if len(p) >= 6 and re.fullmatch(r'[0-9]+', p[0]):
            secs.append((p[1], int(p[3], 16), int(p[2], 16), int(p[5], 16)))
    return secs


def rodata_at(secs, data, addr, lo, hi):
    """What the operand at addr points at, if addr is in .rodata.

    A C string gives its text. A switch jump table (int32 offsets from the
    table to code inside [lo, hi), the function being listed) gives the
    targets as offsets from lo, which a move leaves alone. Anything else
    gives only a mark: its bytes are not compared.
    """
    for name, vma, size, off in secs:
        if name == '.rodata' and vma <= addr < vma + size:
            o = off + addr - vma
            e = data.find(b'\0', o)
            if e < 0:
                e = off + size
            # The table test goes first: its entries are negative offsets
            # back into .text (high bytes 0xff, 0xfe), which no ASCII string
            # gives, while a table can start with a NUL byte and so look
            # like the empty string.
            targets = []
            k = o
            while k + 4 <= off + size and len(targets) < 4096:
                rel = int.from_bytes(data[k:k + 4], 'little', signed=True)
                t = addr + rel
                if not lo <= t < hi:
                    break
                targets.append(t - lo)
                k += 4
            if targets:
                return 'JT' + repr(targets)
            raw = data[o:e]
            if all(32 <= c < 127 or c in (9, 10) for c in raw):
                return 'STR' + repr(raw.decode('ascii'))
            return 'RODATA'
    return None


def listing(binary, fn):
    """Normalized instructions of fn, or None if objdump does not find it."""
    secs = sections(binary)
    with open(binary, 'rb') as f:
        data = f.read()
    out = subprocess.run(['objdump', '-d', '--no-show-raw-insn',
                          '--disassemble=' + fn, binary],
                         capture_output=True, text=True, check=True).stdout
    lines = out.splitlines()
    head = re.compile(r'^[0-9a-f]+ <%s>:' % re.escape(fn))
    start = next((k for k, l in enumerate(lines) if head.match(l)), None)
    if start is None:
        return None
    body = [l for l in lines[start + 1:] if l.strip()]
    addrs = [int(m.group(1), 16) for m in
             (re.match(r'^\s*([0-9a-f]+):', l) for l in body) if m]
    lo = int(lines[start].split()[0], 16)
    hi = (max(addrs) + 16) if addrs else lo
    res = []
    for l in lines[start:]:
        if not l.strip():
            continue
        m = re.search(r'# ([0-9a-f]+) <([^>]*)>', l)
        l = re.sub(r'^\s*[0-9a-f]+:\t', '', l)
        l = re.sub(r'^[0-9a-f]+ <', '<', l)
        if m:
            s = rodata_at(secs, data, int(m.group(1), 16), lo, hi)
            l = re.sub(r'0x[0-9a-f]+\(%rip\)', 'X(%rip)', l)
            note = '# ' + (s if s else '<' + m.group(2) + '>')
            # a function, not a string: the note may hold backslashes
            l = re.sub(r'# .*$', lambda _: note, l)
        l = re.sub(r'\b[0-9a-f]{4,} <', '<', l)
        res.append(l)
    return res


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--dump', metavar='DIR',
                    help='write DIR/<fn>.txt for BINARY instead of comparing')
    ap.add_argument('binaries', nargs='+', metavar='BINARY_OR_FN')
    args = ap.parse_args()

    if args.dump:
        binary, fns = args.binaries[0], args.binaries[1:]
        os.makedirs(args.dump, exist_ok=True)
        for fn in fns:
            res = listing(binary, fn)
            if res is None:
                print(f'{fn}: not found', file=sys.stderr)
                continue
            with open(os.path.join(args.dump, fn + '.txt'), 'w') as f:
                f.write('\n'.join(res) + '\n')
            print(fn, len(res))
        return 0

    if len(args.binaries) < 3:
        ap.error('need OLD_BINARY NEW_BINARY and at least one function')
    old, new, fns = args.binaries[0], args.binaries[1], args.binaries[2:]
    bad = 0
    for fn in fns:
        a, b = listing(old, fn), listing(new, fn)
        if a is None or b is None:
            where = 'old' if a is None else 'new'
            print(f'{fn}: MISSING in {where}')
            bad += 1
        elif a == b:
            print(f'{fn}: same ({len(a)} lines)')
        else:
            print(f'{fn}: DIFFERENT')
            bad += 1
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
