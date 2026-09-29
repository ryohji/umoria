#!/usr/bin/env python3
"""Who calls whom between the object files of src/, read with `nm`.

NOTE: this is only the frame of the layer check planned in
docs/refactoring/layout.md (step L3). The layers are the subdirectories of
src/ (core/, data/, player/, ui/, ...), and those do not exist until step D
moves the files. Until then every unit sits in the one layer "src" and RULES
below is empty, so --check has nothing to enforce. The rules (for example
"core/ must not reach ui/") are to be written here after step D.

What it does now:
  * reads the list of sources from sources.mk (SRCS), so it knows which
    object belongs to which source file and, later, to which directory;
  * runs `nm` on each object and joins every undefined name to the object
    that defines it;
  * prints the dependencies unit by unit (default), layer by layer
    (--layers), or checks them against RULES (--check).

Objects come from the main build (`make`, which leaves *.o in the repository
root) by default; pass --objs tests/build/core to read the objects of
libcore.a instead (`make -f makefile.test libcore`).

Usage:
    python3 scripts/layer_deps.py                   # a.o -> b.o: names...
    python3 scripts/layer_deps.py --unit misc3.o    # only edges out of misc3.o
    python3 scripts/layer_deps.py --callers inventory.o
    python3 scripts/layer_deps.py --layers          # layer -> layer: count
    python3 scripts/layer_deps.py --check           # exit 1 on a violation
"""
import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict

SOURCES_MK = 'sources.mk'

# Layer rules, filled in after step D. Each entry says which layers a layer
# may NOT depend on, e.g.
#     'core': {'data', 'player', 'monster', 'dungeon', 'item', 'store',
#              'combat', 'ui', 'save', 'platform'},
RULES = {}


def read_sources(path=SOURCES_MK):
    """Return {object basename: source path as written in sources.mk}."""
    text = open(path).read()
    m = re.search(r'^SRCS\s*=\s*((?:.*\\\n)*.*)$', text, re.M)
    if not m:
        sys.exit(f'no SRCS in {path}')
    names = re.findall(r'\S+\.c', m.group(1))
    return {os.path.basename(n)[:-2] + '.o': n for n in names}


def layer_of(source):
    """The first directory of the source path, or 'src' while there is none."""
    head = os.path.dirname(source)
    return head.split('/')[0] if head else 'src'


def nm_object(path):
    """Return (defined globals, undefined names) of one object file."""
    out = subprocess.run(['nm', path], capture_output=True, text=True,
                         check=True).stdout
    defined, undefined = set(), set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == 'U':
            undefined.add(parts[1])
        elif len(parts) == 3 and parts[1].isupper() and parts[1] != 'U':
            defined.add(parts[2])
    return defined, undefined


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--objs', default='.',
                    help='directory holding the objects (default: repository root)')
    ap.add_argument('--unit', help='only the edges out of this object')
    ap.add_argument('--callers', help='only the edges into this object')
    ap.add_argument('--layers', action='store_true',
                    help='sum the edges by layer')
    ap.add_argument('--check', action='store_true',
                    help='check the edges against RULES')
    args = ap.parse_args()

    sources = read_sources()
    units = {}
    missing = []
    for obj in sorted(sources):
        path = os.path.join(args.objs, obj)
        if os.path.exists(path):
            units[obj] = nm_object(path)
        else:
            missing.append(obj)
    if not units:
        sys.exit(f'no objects in {args.objs}; build first')
    if missing and args.objs == '.':
        print(f'warning: {len(missing)} objects not built: {" ".join(missing)}',
              file=sys.stderr)

    owner = {}
    for obj, (defined, _) in units.items():
        for name in defined:
            owner.setdefault(name, obj)

    edges = defaultdict(set)
    for obj, (_, undefined) in units.items():
        for name in undefined:
            target = owner.get(name)
            if target and target != obj:
                edges[(obj, target)].add(name)

    if args.check:
        bad = 0
        for (a, b), names in sorted(edges.items()):
            la, lb = layer_of(sources[a]), layer_of(sources[b])
            if lb in RULES.get(la, set()):
                bad += 1
                print(f'{la}/{a} -> {lb}/{b}: {" ".join(sorted(names))}')
        if not RULES:
            print('no layer rules yet (they come after step D); '
                  f'{len(edges)} edges between {len(units)} units')
        sys.exit(1 if bad else 0)

    if args.layers:
        count = defaultdict(int)
        for (a, b) in edges:
            count[(layer_of(sources[a]), layer_of(sources[b]))] += 1
        for (la, lb), n in sorted(count.items()):
            print(f'{la} -> {lb}: {n}')
        return

    for (a, b), names in sorted(edges.items()):
        if args.unit and a != args.unit:
            continue
        if args.callers and b != args.callers:
            continue
        print(f'{a} -> {b}: {" ".join(sorted(names))}')


if __name__ == '__main__':
    main()
