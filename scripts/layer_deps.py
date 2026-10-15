#!/usr/bin/env python3
"""Enforce layer order between object files in src/, checked with `nm`.

Layers (from src/ subdirectories) are ordered: core < model < platform < ui <
world < command < app. A unit may depend only on layers with lower or equal
indices. Within model, sub-libraries (model/data, model/game, model/player,
etc.) have their own rules: model/data and model/game use only core, model/data,
and model/game; others use core, model/data, model/game, and themselves only.

What it does:
  * reads sources from sources.mk (SRCS) to map objects to source files and layers;
  * runs `nm` on each object to find defined globals and undefined names;
  * prints dependencies unit by unit (default), or by layer (--layers),
    or checks against layer order rules (--check).

Use --check with an exceptions file (scripts/layer_exceptions.txt) to allow
known violations. Use --write-exceptions to regenerate the exceptions file
from current objects (sorted).

Objects come from the main build (`make`, repository root) by default; pass
--objs tests/build/core for libcore.a objects instead.

Usage:
    python3 scripts/layer_deps.py                   # a.o -> b.o: names...
    python3 scripts/layer_deps.py --unit spellbook.o  # only edges out of spellbook.o
    python3 scripts/layer_deps.py --callers inventory.o
    python3 scripts/layer_deps.py --layers          # layer -> layer: count
    python3 scripts/layer_deps.py --matrix          # layer counts as table
    python3 scripts/layer_deps.py --check           # exit 1 on violations not in exceptions file
    python3 scripts/layer_deps.py --write-exceptions  # save current violations to exceptions file
"""
import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict

SOURCES_MK = 'sources.mk'
EXCEPTIONS_FILE = 'scripts/layer_exceptions.txt'

# Layer order. A unit at layer i may depend only on layers with index <= i.
LAYER_ORDER = ['core', 'model', 'platform', 'ui', 'world', 'command', 'app']

# Sub-layers within model. Each entry lists which model/* this sub-layer may use.
# model/data and model/game may use each other and core (if that edge exists today).
# Others may use model/data, model/game, and themselves only.
MODEL_RULES = {
    'data': {'core', 'model/data', 'model/game'},
    'game': {'core', 'model/data', 'model/game'},
    # The floor and the shops hold items.
    'dungeon': {'core', 'model/data', 'model/game', 'model/item', 'model/dungeon'},
    'item': {'core', 'model/data', 'model/game', 'model/item'},
    'monster': {'core', 'model/data', 'model/game', 'model/monster'},
    'player': {'core', 'model/data', 'model/game', 'model/player'},
    'store': {'core', 'model/data', 'model/game', 'model/item', 'model/store'},
}


# world/ is one library whose files call each other in a loop, but the files
# fall into four tiers that only call down: what is drawn, the rules on items
# and the character, what lies on the floor and who lives or dies, and the
# actions of the player and the monsters. A world/ file in tier i may call
# world/ files in tiers <= i. Every world/ file must be listed here.
WORLD_TIERS = [
    ('view', 'map_view status_line lighting'),
    ('rules', 'desc inscription item_picker inven_ops player_bonuses stat_ops level_ops '
              'spellbook rest_command stat_loss food_ops item_learn device_use store_price'),
    ('floor and life', 'object_place object_alloc monster_death monster_place monster_damage '
                       'hit_rolls player_damage item_enchant monster_breeding store_stock '
                       'store_haggle level_exit generate'),
    ('actions', 'creature monster_melee player_melee projectiles spells traps search '
                'terrain_commands player_move run_path throw'),
]
WORLD_TIER = {u + '.o': i for i, (_, us) in enumerate(WORLD_TIERS) for u in us.split()}


def read_sources(path=SOURCES_MK):
    """Return {object basename: source path as written in sources.mk}."""
    text = open(path).read()
    m = re.search(r'^SRCS\s*=\s*((?:.*\\\n)*.*)$', text, re.M)
    if not m:
        sys.exit(f'no SRCS in {path}')
    names = re.findall(r'\S+\.c', m.group(1))
    return {os.path.basename(n)[:-2] + '.o': n for n in names}


def layer_of(source):
    """First directory in source path; 'model/<sublib>' for model/* files.

    Examples: 'core/foo.c' -> 'core', 'model/data/bar.c' -> 'model/data',
              'ui/baz.c' -> 'ui', 'main.c' -> 'app'.
    """
    parts = source.split('/')
    if not parts[0]:
        return 'app'  # shouldn't happen
    if parts[0] == 'model' and len(parts) > 1:
        return f'model/{parts[1]}'
    return parts[0] if parts[0] != '' else 'app'


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


def layer_index(layer):
    """Return the index of a layer (or 'model/<sublib>'), or None if unknown.

    'model/data' returns the index of 'model'.
    """
    base = layer.split('/')[0] if '/' in layer else layer
    try:
        return LAYER_ORDER.index(base)
    except ValueError:
        return None


def is_violation(from_layer, to_layer):
    """Return True if from_layer may NOT depend on to_layer.

    Returns True for violations; False if the dependency is allowed.
    """
    # model/* sub-layer rules
    if from_layer.startswith('model/'):
        sub = from_layer.split('/')[1]
        allowed = MODEL_RULES.get(sub, set())
        return to_layer not in allowed

    # Main layer rules: a unit at layer i may depend only on layers with index <= i
    from_idx = layer_index(from_layer)
    to_idx = layer_index(to_layer)
    if from_idx is None or to_idx is None:
        return False
    return to_idx > from_idx


def read_exceptions(path=EXCEPTIONS_FILE):
    """Return set of exception edges as tuples: (from_obj, to_obj, frozenset(names))."""
    exceptions = {}
    if not os.path.exists(path):
        return exceptions
    for line in open(path):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        # Format: "from.o -> to.o: name1 name2 ..."
        m = re.match(r'^(\S+)\s+->\s+(\S+):\s*(.*)$', line)
        if m:
            from_obj, to_obj, names_str = m.groups()
            names = frozenset(names_str.split()) if names_str else frozenset()
            exceptions[(from_obj, to_obj)] = names
    return exceptions


def write_exceptions(edges, sources, path=EXCEPTIONS_FILE):
    """Write all current violations to exceptions file, sorted."""
    violations = []
    for (a, b), names in sorted(edges.items()):
        la, lb = layer_of(sources[a]), layer_of(sources[b])
        if is_violation(la, lb):
            violations.append((a, b, sorted(names)))

    with open(path, 'w') as f:
        f.write('# Layer dependency exceptions (from_unit.o -> to_unit.o: symbols)\n')
        for a, b, names in violations:
            f.write(f'{a} -> {b}: {" ".join(names)}\n')


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
                    help='fail on violations not in exceptions file')
    ap.add_argument('--write-exceptions', action='store_true',
                    help='write current violations to exceptions file')
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

    if args.write_exceptions:
        write_exceptions(edges, sources, EXCEPTIONS_FILE)
        print(f'Wrote exceptions to {EXCEPTIONS_FILE}')
        return

    if args.check:
        exceptions = read_exceptions(EXCEPTIONS_FILE)
        violations = {}
        remaining_exceptions = set(exceptions.keys())

        # Find violations and check against exceptions
        bad = 0
        for (a, b), names in sorted(edges.items()):
            la, lb = layer_of(sources[a]), layer_of(sources[b])
            if is_violation(la, lb):
                exc_names = exceptions.get((a, b), frozenset())
                remaining_exceptions.discard((a, b))
                if (a, b) not in exceptions:
                    bad += 1
                    print(f'{a} -> {b}: {" ".join(sorted(names))}')
                elif not names <= exc_names:
                    # Violation with symbols not in the exception
                    new_symbols = sorted(names - exc_names)
                    bad += 1
                    print(f'{a} -> {b}: new symbols {" ".join(new_symbols)}')

        # world/ tiers
        for obj, src in sorted(sources.items()):
            if layer_of(src) == 'world' and obj in units and obj not in WORLD_TIER:
                bad += 1
                print(f'{obj}: world/ file without a tier (add it to WORLD_TIERS)')
        for (a, b), names in sorted(edges.items()):
            if a in WORLD_TIER and b in WORLD_TIER and WORLD_TIER[b] > WORLD_TIER[a]:
                if (a, b) in exceptions and names <= exceptions[(a, b)]:
                    remaining_exceptions.discard((a, b))
                    continue
                bad += 1
                print(f'{a} ({WORLD_TIERS[WORLD_TIER[a]][0]}) -> {b} '
                      f'({WORLD_TIERS[WORLD_TIER[b]][0]}): {" ".join(sorted(names))}')

        # Check for obsolete exceptions
        for (a, b) in remaining_exceptions:
            print(f'# obsolete exception: {a} -> {b}')

        # Summary: count remaining exceptions and their symbols
        remaining_symbols = sum(len(names) for names in exceptions.values())

        print(f'{"FAIL" if bad else "OK"}: {len(edges)} edges between {len(units)} units; '
              f'{len(exceptions)} known exceptions remain '
              f'({remaining_symbols} symbols)')
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
