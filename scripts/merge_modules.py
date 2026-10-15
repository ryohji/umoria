#!/usr/bin/env python3
"""Merge several small modules (x.c + x.h) into one, mechanically.

usage: merge_modules.py REPO DIR NEW OLD1 OLD2 ...
  DIR  directory under src/ (e.g. model/player)
  NEW  base name of the merged module
The bodies are concatenated unchanged; includes are unioned; every
#include "OLDk.h" in src/ and tests/ becomes #include "NEW.h" (once per file);
sources.mk gets NEW.c at the position of OLD1.c and loses the others.
A file-scope static whose name appears in more than one old file is renamed
<old>_<name> in each (two statics of the same name in one translation unit
would silently become one variable). Check the result with scripts/cmp_all.sh.
"""
import os, re, subprocess, sys

repo, d, new, olds = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:]
src = os.path.join(repo, 'src', d)
old_h = {o + '.h' for o in olds}
INC = re.compile(r'^\s*#\s*include\s+([<"][^">]+[">])')


def split_c(text):
    """(leading comment block, include lines, body)."""
    lines = text.split('\n')
    i = 0
    lead = []
    while i < len(lines) and (lines[i].startswith('//') or lines[i].strip() == ''):
        lead.append(lines[i]); i += 1
    incs = []
    while i < len(lines):
        s = lines[i].strip()
        if INC.match(lines[i]) or s == '':
            if s:
                incs.append(lines[i].strip())
            i += 1
        else:
            break
    return lead, incs, '\n'.join(lines[i:]).strip('\n')


def split_h(text, name):
    lines = text.split('\n')
    g = next(k for k, l in enumerate(lines) if l.startswith('#define ') and l.split()[1].endswith('_H'))
    e = max(k for k, l in enumerate(lines) if l.startswith('#endif'))
    inner = lines[g + 1:e]
    incs = [l.strip() for l in inner if INC.match(l)]
    inner = [l for l in inner if not INC.match(l)]
    return incs, '\n'.join(inner).strip('\n')


def license_of(lead):
    out = []
    for l in lead:
        if l.strip() == '' and out and any('for further details' in x for x in out):
            break
        out.append(l)
    return out


def dedupe(incs, self_h):
    seen, out = set(), []
    for inc in incs:
        m = INC.match(inc)
        name = m.group(1)[1:-1]
        if name in old_h:
            inc, name = f'#include "{self_h}"', self_h
        if name in seen:
            continue
        seen.add(name); out.append(inc)
    return out


STATIC = re.compile(r'^static\b[^=;{]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*(?:=[^;]*)?;|^static\b[^=;{(]*?\b([A-Za-z_]\w*)\s*\(', re.M)


def statics(text):
    return {a or b for a, b in STATIC.findall(text)}


texts = {o: (open(os.path.join(src, o + '.c')).read() if os.path.exists(os.path.join(src, o + '.c')) else '') for o in olds}
seen = {}
for o in olds:
    for n in statics(texts[o]):
        seen.setdefault(n, []).append(o)
for n, owners in seen.items():
    if len(owners) > 1:
        for o in owners:
            short = o[len('player_'):] if o.startswith('player_') else o
            texts[o] = re.sub(r'\b' + re.escape(n) + r'\b', f'{short}_{n}', texts[o])
            print(f'renamed static {n} in {o}.c -> {short}_{n}')

lic = None
c_incs, c_bodies, h_incs, h_bodies = [], [], [], []
for o in olds:
    ct = texts[o]
    if ct:
        lead, incs, body = split_c(ct)
        lic = lic or license_of(lead)
        c_incs += incs
        c_bodies.append(body)
    hp = os.path.join(src, o + '.h')
    if os.path.exists(hp):
        hi, hb = split_h(open(hp).read(), o)
        h_incs += hi
        h_bodies.append(hb)

guard = new.upper() + '_H'
c_incs = dedupe(c_incs, new + '.h')
# keep the self include where the first old self include was
h_incs = [i for i in dedupe(h_incs, new + '.h') if f'"{new}.h"' not in i]

with open(os.path.join(src, new + '.c'), 'w') as f:
    f.write('\n'.join(lic) + '\n\n' + '\n'.join(c_incs) + '\n\n' + '\n\n'.join(c_bodies) + '\n')
with open(os.path.join(src, new + '.h'), 'w') as f:
    f.write('\n'.join(lic) + '\n\n' + f'#ifndef {guard}\n#define {guard}\n\n')
    if h_incs:
        f.write('\n'.join(h_incs) + '\n\n')
    f.write('\n\n'.join(h_bodies) + f'\n\n#endif // {guard}\n')

# includes everywhere
for top in ('src', 'tests'):
    for root, _, files in os.walk(os.path.join(repo, top)):
        for fn in files:
            if not fn.endswith(('.c', '.h')):
                continue
            p = os.path.join(root, fn)
            if os.path.samefile(p, os.path.join(src, new + '.h')) or os.path.samefile(p, os.path.join(src, new + '.c')):
                continue
            lines = open(p).read().split('\n')
            out, done, changed = [], False, False
            for l in lines:
                m = INC.match(l)
                if m and m.group(1)[1:-1] in old_h:
                    changed = True
                    if not done:
                        out.append(f'#include "{new}.h"'); done = True
                    continue
                out.append(l)
            if changed:
                open(p, 'w').write('\n'.join(out))

# sources.mk
mk = os.path.join(repo, 'sources.mk')
t = open(mk).read()
first = next(f'{d}/{o}.c' for o in olds if texts[o])
assert first in t, first
t = t.replace(first, f'{d}/{new}.c', 1)
for o in [o for o in olds if texts[o] and f'{d}/{o}.c' != first]:
    t = re.sub(r'(?<=\t)' + re.escape(f'{d}/{o}.c') + r'[ ]*', '', t)
    t = re.sub(r'[ ]+' + re.escape(f'{d}/{o}.c') + r'(?=[ \t\\\n])', '', t)
t = re.sub(r'\n[ \t]*\\\n', '\n', t)  # lines left with only a continuation
open(mk, 'w').write(t)

rm = [f'src/{d}/{o}.{x}' for o in olds for x in ('c', 'h') if os.path.exists(os.path.join(src, f'{o}.{x}'))]
subprocess.run(['git', '-C', repo, 'rm', '-qf'] + rm, check=True)
print('merged', len(olds), 'modules into', new)
