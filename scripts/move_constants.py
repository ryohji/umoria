#!/usr/bin/env python3
"""Move the #defines of src/constant.h that only one place uses to that place.

usage: python3 scripts/move_constants.py [--dry-run]

A define used by exactly one .c file (and by no test, header or other define)
moves to the top of that .c file. A define used only inside one library that
has an internal header (sources.mk INTERNAL_LIBS) moves to that header. All
others stay. A section heading (the comment lines above a group of defines)
goes with its defines, and stays in constant.h too while any of them remains.
"""
import os
import re
import sys

HDR = 'src/constant.h'
TOKEN = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
IDENT = re.compile(r'\b[A-Za-z_]\w*\b')
DEFINE = re.compile(r'^\s*#\s*define\s+([A-Za-z_]\w*)')
dry = '--dry-run' in sys.argv

internal_libs = re.search(r'^INTERNAL_LIBS\s*=\s*(.*)$', open('sources.mk').read(), re.M).group(1).split()


def ids_of(path):
    return set(IDENT.findall(TOKEN.sub(' ', open(path).read())))


files = {}
for top in ('src', 'tests'):
    for root, _, fs in os.walk(top):
        if 'build' in root:
            continue
        for f in fs:
            if f.endswith(('.c', '.h')):
                p = os.path.join(root, f)
                if p != HDR:
                    files[p] = ids_of(p)


def library(p):
    parts = p.split('/')
    if parts[0] != 'src' or len(parts) < 3:
        return None
    return parts[1]


lines = open(HDR).read().split('\n')
g = next(k for k, l in enumerate(lines) if l.startswith('#define CONSTANT_H'))
e = max(k for k, l in enumerate(lines) if l.startswith('#endif'))

# sections: heading comments + define lines, separated by blank lines
sections, cur = [], {'head': [], 'defs': [], 'other': [], 'items': []}
depth = 0
for k in range(g + 1, e):
    l = lines[k]
    s = l.strip()
    if re.match(r'#\s*if', s):
        depth += 1
    if depth or re.match(r'#\s*(if|else|elif|endif)', s):
        cur['other'].append(l)
        cur['items'].append(l)
        if re.match(r'#\s*endif', s):
            depth -= 1
        continue
    if s == '':
        if cur['head'] or cur['defs'] or cur['other']:
            sections.append(cur)
        cur = {'head': [], 'defs': [], 'other': [], 'items': []}
        continue
    if DEFINE.match(l):
        cur['defs'].append(l)
        cur['items'].append(l)
    elif s.startswith('//') and not cur['defs'] and not cur['other']:
        cur['head'].append(l)
    else:
        cur['other'].append(l)
        cur['items'].append(l)
if cur['head'] or cur['defs'] or cur['other']:
    sections.append(cur)

# names referenced by other definitions in constant.h or by types.h/config.h
hdr_code = TOKEN.sub(' ', '\n'.join(lines))
refs_in_hdr = {}
for l in lines:
    m = DEFINE.match(l)
    if m:
        body = TOKEN.sub(' ', l)[m.end():]
        for n in IDENT.findall(body):
            refs_in_hdr.setdefault(n, set()).add(m.group(1))


def destination(name):
    if name in refs_in_hdr:
        return None
    users = [p for p, ids in files.items() if name in ids]
    if not users:
        return None
    if any(not p.startswith('src/') for p in users):
        return None  # a test uses it: keep it where every test sees it
    headers = [p for p in users if p.endswith('.h')]
    if headers:
        libs = {library(p) for p in users}
        if len(libs) == 1 and next(iter(libs)) in internal_libs and \
                all('/internal/' in p for p in headers):
            return ('internal', next(iter(libs)))
        return None
    if len(users) == 1:
        return ('file', users[0])
    libs = {library(p) for p in users}
    if len(libs) == 1 and next(iter(libs)) in internal_libs:
        return ('internal', next(iter(libs)))
    return None


moves = {}       # destination -> list of blocks (lines)
keep = []
count = {'file': 0, 'internal': 0, 'kept': 0}
for sec in sections:
    by_dest, stay = {}, []
    for d in sec['items']:
        m = DEFINE.match(d)
        dest = destination(m.group(1)) if m and not sec['other'] else None
        if dest:
            by_dest.setdefault(dest, []).append(d)
            count[dest[0]] += 1
        else:
            stay.append(d)
            if m:
                count['kept'] += 1
    for dest, defs in by_dest.items():
        moves.setdefault(dest, []).append(sec['head'] + defs)
    if stay or not sec['items']:
        keep.append(sec['head'] + stay)

print(count)
for dest, blocks in sorted(moves.items()):
    print(dest, sum(len([l for l in b if DEFINE.match(l)]) for b in blocks))
if dry:
    sys.exit(0)

# write constant.h
out = lines[:g + 1] + ['']
for b in keep:
    out += b + ['']
out += lines[e:]
open(HDR, 'w').write(re.sub(r'\n{3,}', '\n\n', '\n'.join(out)))


def insert_after_includes(path, block):
    t = open(path).read().split('\n')
    last = max(k for k, l in enumerate(t) if re.match(r'\s*#\s*include\b', l))
    t = t[:last + 1] + [''] + block + t[last + 1:]
    open(path, 'w').write(re.sub(r'\n{3,}', '\n\n', '\n'.join(t)))


for (kind, where), blocks in moves.items():
    block = []
    for b in blocks:
        block += b + ['']
    block = block[:-1]
    if kind == 'file':
        insert_after_includes(where, block)
    else:
        p = f'src/{where}/internal/{where}_internal.h'
        t = open(p).read()
        k = t.rindex('#endif')
        t = t[:k].rstrip('\n') + '\n\n// --- constants ---\n' + '\n'.join(block) + '\n\n' + t[k:]
        open(p, 'w').write(t)
        names = {DEFINE.match(l).group(1) for l in block if DEFINE.match(l)}
        inc = f'#include "{where}_internal.h"'
        for q, ids in files.items():
            if q.startswith(f'src/{where}/') and q.endswith('.c') and ids & names:
                if inc not in open(q).read():
                    insert_after_includes(q, [inc])
