#!/bin/bash
# HANDOVER section 2, checks 1-4 and the golden runs, from clean.
# usage: scripts/check.sh [WORKTREE]
# Prints one line per check; exits 1 if any of them is not as it must be.
cd "${1:-.}" || exit 1
bad=0
make clean >/dev/null
w=$(make -j8 2>&1 | grep -cE 'warning:|error:')
echo "1. warnings: $w"; [ "$w" -eq 0 ] || bad=1
rm -rf tests/build
t=$(make -j8 -f makefile.test 2>&1 | grep -E "^[0-9]+ tests|RESULT|Segmentation" \
    | awk '/ tests,/{s+=$1; f+=$5; n++} /RESULT/{r=$2} /Segm/{g++} END {print s" tests, "n" executables, failed="f", "r", segfaults="g+0}')
echo "2. tests: $t"
case "$t" in *"failed=0, GREEN, segfaults=0"*) ;; *) bad=1 ;; esac
g=$(python3 scripts/globals.py --check | tail -1); echo "3. globals: $g"; case "$g" in OK*) ;; *) bad=1 ;; esac
l=$(python3 scripts/layer_deps.py --check | tail -1); echo "3. layers: $l"; case "$l" in OK*) ;; *) bad=1 ;; esac
h=$(make -f makefile.test headers 2>&1 | tail -2 | tr '\n' ' '); echo "3. headers: $h"; case "$h" in *"core: compiles"*) ;; *) bad=1 ;; esac
# Only model/data/monsters.c reads or makes the index inside the creature
# iterators (creature_iterator.place, creature_rev_iterator.base).
it=$(grep -rnE '\.(place|base)\b' src --include='*.[ch]' | grep -v '^src/model/data/monsters\.c:' | grep -cvE '^[^:]*:[0-9]+:\s*//')
echo "3. iterator index outside monsters.c: $it"; [ "$it" -eq 0 ] || bad=1
touch scores.dat
p=$(printf '\n\n\n\n' | timeout 10 script -qc "TERM=xterm ./umoria /tmp/check_probe.$$.sav" /dev/null | tr -d '\r' | grep -c "Choose a race")
rm -f scores.dat /tmp/check_probe.$$.sav
echo "4. startup: $p"; [ "$p" -ge 1 ] || bad=1
gd=$(make -f makefile.test golden 2>&1 | grep '^golden:' | tail -1); echo "4. golden: $gd"; [ "$gd" = "golden: GREEN" ] || bad=1
[ $bad -eq 0 ] && echo "ALL OK" || echo "NOT OK"
exit $bad
