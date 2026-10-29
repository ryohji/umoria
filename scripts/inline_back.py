#!/usr/bin/env python3
"""Check that functions were moved out of a C file without changing a character.

usage: scripts/inline_back.py BASE NEW FUNC...

BASE is the file before the change (for example `git show HEAD:src/app/dungeon.c`
saved to a file), NEW the file after it. Each FUNC is a function of NEW that
was cut out of a longer one. The tool puts the body of each FUNC back where it
is called, takes out its definition and its prototype, moves the comment lines
right above the definition to above the call, and prints the difference from
BASE (none: exit 0).

A call is a line of its own: `FUNC();`, `x = FUNC();` or `return FUNC();`,
with arguments only when each is a plain name that is also the parameter's
name (`x = FUNC(a, b);` for `FUNC(int a, bool b)`), so the body reads the same.
The body goes back with its indentation moved from 4 spaces to the call's. A
trailing `return x;` of the body is dropped when the call was `x = FUNC();`
(and kept when the call itself was a `return`). Whatever else differs (a
local declared in the new function, a parameter) shows up in the difference
and has to be read.
"""
import re
import sys


def cut_function(lines, name):
    head = re.compile(r'^(?:static\s+)?[A-Za-z_][\w\s\*]*\b' + re.escape(name) + r'\(([^)]*)\)\s*\{$')
    for start, line in enumerate(lines):
        m = head.match(line)
        if m:
            break
    else:
        sys.exit(f"inline_back: no definition of {name}")
    params = [] if m.group(1).strip() in ('', 'void') else [re.findall(r'\w+', p)[-1] for p in m.group(1).split(',')]
    end = start + 1
    while lines[end] != '}':
        end += 1
    body = lines[start + 1:end]
    first = start
    while first > 0 and lines[first - 1].startswith('//'):
        first -= 1
    comments = lines[first:start]
    rest = lines[:first] + lines[end + 1:]
    # the blank line that separated the definition from what came before
    if first > 0 and first < len(rest) and rest[first - 1] == '' and rest[first] == '':
        del rest[first]
    proto = re.compile(r'^(?:static\s+)?[A-Za-z_][\w\s\*]*\b' + re.escape(name) + r'\([^)]*\);$')
    rest = [l for l in rest if not proto.match(l)]
    return comments, body, rest, params


def reindent(body, indent):
    out = []
    for line in body:
        if line == '' or line.startswith('#'):  # preprocessor lines keep column 0
            out.append(line)
        elif line.startswith('    '):
            out.append(indent + line[4:])
        else:
            sys.exit(f"inline_back: body line not indented: {line!r}")
    return out


def inline(lines, name, comments, body, params):
    call = re.compile(r'^(\s*)(?:(\w+) = |(return) )?' + re.escape(name) + r'\(([^()]*)\);$')
    out, found = [], 0
    for line in lines:
        m = call.match(line)
        if not m:
            out.append(line)
            continue
        found += 1
        indent, target, ret = m.group(1), m.group(2), m.group(3)
        args = [a.strip() for a in m.group(4).split(',')] if m.group(4).strip() else []
        if args != params:
            sys.exit(f"inline_back: {name}({', '.join(args)}) does not pass its parameters ({', '.join(params)}) by name")
        b = list(body)
        if target and b and re.match(r'^    return ' + re.escape(target) + r';$', b[-1]):
            b = b[:-1]
        out.extend(indent + c for c in comments)
        out.extend(reindent(b, indent))
    if found == 0:
        sys.exit(f"inline_back: no call of {name}")
    return out


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    base = open(sys.argv[1]).read().split('\n')
    new = open(sys.argv[2]).read().split('\n')
    for name in sys.argv[3:]:
        comments, body, new, params = cut_function(new, name)
        new = inline(new, name, comments, body, params)
    import difflib
    diff = list(difflib.unified_diff(base, new, 'base', 'inlined', lineterm='', n=1))
    for line in diff:
        print(line)
    if diff:
        sys.exit(1)
    print(f"same: {len(base)} lines, {len(sys.argv) - 3} functions put back")


if __name__ == '__main__':
    main()
