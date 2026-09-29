#!/usr/bin/env python3
"""Which object files from libcore.a did each test executable pull in?

makefile.test links every test as

    tests/<name>.c  <its tests/ helpers>  tests/build/libcore.a

and asks the linker for a map (tests/build/<name>.map). The archive has no
hand-written list: the linker takes a member only when it defines a name that
is still undefined. This script reads the "Archive member included to satisfy
reference by file (symbol)" section of each map and prints the members.

Usage:
    python3 scripts/link_units.py                  # every tests/build/*.map
    python3 scripts/link_units.py tests/build/objdes_test.map
    python3 scripts/link_units.py --why objdes_test   # which name pulled each member
    python3 scripts/link_units.py --who misc3.o    # which tests pull misc3.o
    python3 scripts/link_units.py --count          # one line per test: the count
    python3 scripts/link_units.py --shadows        # see below

--shadows lists, for each test, the global names that the test side (the test
itself, its stubs and fixtures, or a src/*.c it #includes whole) defines while
an archive member that was *not* pulled defines them too. Those are the places
where a stand-in covers the real thing. If a stand-in is deleted, the linker
does not stop with "undefined reference" as it did with the old hand-written
recipes; it quietly pulls the real member instead. After removing a stand-in,
run this script and check that the member it covered now shows up (or that it
does not, if that is what you meant).

Run `make -f makefile.test` first; the maps are written by the link step.
"""
import argparse
import glob
import os
import re
import subprocess
import sys

MAP_GLOB = 'tests/build/*.map'
SECTION = 'Archive member included to satisfy reference by file (symbol)'
MEMBER = re.compile(r'^(\S+?\.a)\(([^)]+)\)(?:\s+(\S+)\s+\((.+)\))?\s*$')
REFERENCE = re.compile(r'^\s+(\S+)\s+\((.+)\)\s*$')


def read_map(path):
    """Return (archive path or None, {member: [(referrer, symbol), ...]})."""
    text = open(path).read()
    start = text.find(SECTION)
    if start < 0:
        return None, {}
    body = text[start + len(SECTION):]
    # The section ends at the next header that starts in column 0.
    end = re.search(r'\n\n(?=\S)', body.lstrip('\n'))
    body = body.lstrip('\n')
    if end:
        body = body[:end.start()]
    archive = None
    members = {}
    current = None
    for line in body.split('\n'):
        m = MEMBER.match(line)
        if m:
            archive = m.group(1)
            current = m.group(2)
            members.setdefault(current, [])
            if m.group(3):
                members[current].append((m.group(3), m.group(4)))
            continue
        r = REFERENCE.match(line)
        if r and current:
            members[current].append((r.group(1), r.group(2)))
    return archive, members


def test_name(path):
    return os.path.basename(path)[:-len('.map')]


def nm_defined(path):
    """Global names defined by an executable: {name}."""
    out = subprocess.run(['nm', '--defined-only', path], capture_output=True,
                         text=True, check=True).stdout
    names = set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[1].isupper():
            names.add(parts[2])
    return names


def nm_archive(path):
    """Global names defined by each archive member: {member: {name}}."""
    out = subprocess.run(['nm', '--defined-only', path], capture_output=True,
                         text=True, check=True).stdout
    defs = {}
    member = None
    for line in out.splitlines():
        m = re.match(r'^(\S+\.o):$', line)
        if m:
            member = m.group(1)
            defs.setdefault(member, set())
            continue
        parts = line.split()
        if member and len(parts) == 3 and parts[1].isupper():
            defs[member].add(parts[2])
    return defs


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('maps', nargs='*', help='map files (default: %s)' % MAP_GLOB)
    ap.add_argument('--why', metavar='TEST',
                    help='show which reference pulled each member of TEST')
    ap.add_argument('--who', metavar='MEMBER',
                    help='list the tests that pull MEMBER (e.g. misc3.o)')
    ap.add_argument('--count', action='store_true',
                    help='print only the number of members per test')
    ap.add_argument('--shadows', action='store_true',
                    help='list test-side definitions that cover archive members')
    args = ap.parse_args()

    maps = args.maps or sorted(glob.glob(MAP_GLOB))
    if args.why:
        maps = [m for m in maps if test_name(m) == args.why]
    if not maps:
        sys.exit('no map files; run `make -f makefile.test` first')

    archive_defs = None
    for path in maps:
        name = test_name(path)
        archive, members = read_map(path)
        if args.who:
            if args.who in members:
                print(name)
            continue
        if args.why:
            for member in sorted(members):
                refs = ', '.join(f'{sym} <- {os.path.basename(ref)}'
                                 for ref, sym in members[member])
                print(f'{member}: {refs}')
            continue
        if args.count:
            print(f'{name} {len(members)}')
            continue
        if args.shadows:
            exe = path[:-len('.map')]
            if archive is None:
                archive = 'tests/build/libcore.a'
            if archive_defs is None:
                archive_defs = nm_archive(archive)
            defined = nm_defined(exe)
            lines = []
            for member in sorted(archive_defs):
                if member in members:
                    continue
                covered = sorted(defined & archive_defs[member])
                if covered:
                    lines.append(f'  {member}: {" ".join(covered)}')
            print(f'{name}: {len(lines)} members covered')
            for line in lines:
                print(line)
            continue
        print(f'{name} ({len(members)}): {" ".join(sorted(members))}')


if __name__ == '__main__':
    main()
