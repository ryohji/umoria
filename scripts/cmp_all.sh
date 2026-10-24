#!/bin/bash
# Compare the machine code of every function between a base commit and a
# working tree, and check that no data object was merged away.
#
# usage: scripts/cmp_all.sh BASE_SHA WORKTREE OUTDIR
#
# Both sides are built with -O2 -fno-inline -fno-ipa-ra (WORKTREE ends with
# `make clean`). Functions that differ only in <symbol> comments are reported
# as "same except symbol comments" (renamed statics); the rest as REALLY
# DIFFERENT. The last line compares the (type, size) multiset of data symbols:
# two file statics with the same name merged into one translation unit become
# one variable without a warning, and this is where that shows.
set -e
BASE=$1; HEAD_DIR=$2; OUT=$3
mkdir -p $OUT
REPO=$(git -C $HEAD_DIR rev-parse --show-toplevel)
rm -rf $OUT/base; git -C $REPO worktree prune; git -C $REPO worktree add -q --detach $OUT/base $BASE
FL='OPT_FLAGS=-O2 -fno-inline -fno-ipa-ra'
(cd $OUT/base || exit 1; make clean >/dev/null; make -j8 "$FL" >/dev/null 2>&1; cp umoria $OUT/umoria.base)
(cd $HEAD_DIR || exit 1; make clean >/dev/null; make -j8 "$FL" >/dev/null 2>&1; cp umoria $OUT/umoria.head; make clean >/dev/null)
git -C $REPO worktree remove --force $OUT/base
nm $OUT/umoria.base | awk '$2~/[Tt]/{print $3}' | sort -u > $OUT/f.base
nm $OUT/umoria.head | awk '$2~/[Tt]/{print $3}' | sort -u > $OUT/f.head
echo "only in base: $(comm -23 $OUT/f.base $OUT/f.head | tr '\n' ' ')"
echo "only in head: $(comm -13 $OUT/f.base $OUT/f.head | tr '\n' ' ')"
comm -12 $OUT/f.base $OUT/f.head | grep -v '^_\|^\.\|frame_dummy\|register_tm\|deregister_tm' > $OUT/f.common
python3 $REPO/scripts/dis_compare.py $OUT/umoria.base $OUT/umoria.head $(cat $OUT/f.common) > $OUT/result.txt || true
echo "common: $(wc -l < $OUT/f.common)  same: $(grep -c ' same' $OUT/result.txt)  different: $(grep -vc ' same' $OUT/result.txt)"
grep -v ' same' $OUT/result.txt | head -30
# second pass: differences only in symbol-name comments (renamed statics)
D=$(grep -v ' same' $OUT/result.txt | sed 's/:.*//')
if [ -n "$D" ]; then
  rm -rf $OUT/db $OUT/dh
  python3 $REPO/scripts/dis_compare.py --dump $OUT/db $OUT/umoria.base $D >/dev/null
  python3 $REPO/scripts/dis_compare.py --dump $OUT/dh $OUT/umoria.head $D >/dev/null
  for f in $D; do
    if diff -q <(sed 's/#.*//' $OUT/db/$f.txt) <(sed 's/#.*//' $OUT/dh/$f.txt) >/dev/null; then echo "$f: same except symbol comments"; else echo "$f: REALLY DIFFERENT"; fi
  done | sort | uniq -c -f1 | sed -n '1,40p'
  echo "really different: $(for f in $D; do diff -q <(sed 's/#.*//' $OUT/db/$f.txt) <(sed 's/#.*//' $OUT/dh/$f.txt) >/dev/null || echo $f; done | tr '\n' ' ')"
fi
$REPO/scripts/check_statics.sh $OUT/umoria.base $OUT/umoria.head
