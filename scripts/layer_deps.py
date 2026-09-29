#!/usr/bin/env python3
"""Who calls whom between the object files of src/, read with `nm`.

NOTE: this is the frame of the layer check planned in
docs/refactoring/layout.md (step L3). The layers are the subdirectories of
src/ (core/, data/, player/, ui/, ...), read from the paths in sources.mk;
a file directly in src/ (main.c, dungeon.c, the numbered files until step R)
is in the layer "src". Step D (#53) moves the files, and --matrix shows who
calls whom between the layers. RULES below is still empty, so --check has
nothing to enforce: the rules (for example "core/ must not reach ui/") are
to be decided from that matrix and written here.

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
    python3 scripts/layer_deps.py --matrix          # the same as a table
    python3 scripts/layer_deps.py --check           # exit 1 on a violation
"""
import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict

SOURCES_MK = 'sources.mk'

# Layer rules, to be decided from --matrix after step D. Each entry says
# which layers a layer may NOT depend on, e.g.
#     'core': {'data', 'player', 'monster', 'dungeon', 'item', 'store',
#              'combat', 'ui', 'save', 'platform'},
RULES = {}

# The order of the rows and columns of --matrix (docs/refactoring/layout.md).
# A layer that is not here (a new directory) is appended in name order.
LAYER_ORDER = ['src', 'core', 'data', 'player', 'monster', 'dungeon', 'item',
               'store', 'combat', 'ui', 'save', 'platform']


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


def print_matrix(count, layers):
    """Rows call columns; a cell is the number of unit -> unit edges."""
    order = [l for l in LAYER_ORDER if l in layers]
    order += sorted(layers - set(order))
    width = max(len(l) for l in order + ['total'])
    cell = max(5, max((len(str(n)) for n in count.values()), default=1) + 1)
    print('rows call columns; a cell counts unit -> unit edges')
    print(' ' * width + ''.join(f'{l[:cell - 1]:>{cell}}' for l in order)
          + f'{"total":>{cell + 1}}')
    for la in order:
        row = [count.get((la, lb), 0) for lb in order]
        print(f'{la:<{width}}'
              + ''.join(f'{(n if n else "."):>{cell}}' for n in row)
              + f'{sum(row):>{cell + 1}}')
    cols = [sum(count.get((la, lb), 0) for la in order) for lb in order]
    print(f'{"total":<{width}}' + ''.join(f'{n:>{cell}}' for n in cols)
          + f'{sum(cols):>{cell + 1}}')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--objs', default='.',
                    help='directory holding the objects (default: repository root)')
    ap.add_argument('--unit', help='only the edges out of this object')
    ap.add_argument('--callers', help='only the edges into this object')
    ap.add_argument('--layers', action='store_true',
                    help='sum the edges by layer')
    ap.add_argument('--matrix', action='store_true',
                    help='print the layer -> layer edge counts as a table')
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
            print('no layer rules yet (see --matrix); '
                  f'{len(edges)} edges between {len(units)} units')
        sys.exit(1 if bad else 0)

    if args.layers or args.matrix:
        count = defaultdict(int)
        for (a, b) in edges:
            count[(layer_of(sources[a]), layer_of(sources[b]))] += 1
        if args.matrix:
            print_matrix(count, {layer_of(sources[o]) for o in units})
            return
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
