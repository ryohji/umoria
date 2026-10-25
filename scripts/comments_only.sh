#!/bin/bash
# Did FILE change only in comments and whitespace since REV?
# usage: scripts/comments_only.sh REPO REV FILE...
cd "$1" || exit 1; REV=$2; shift 2; rc=0
for f in "$@"; do
  strip(){ gcc -fpreprocessed -dD -E -P -x c - 2>/dev/null | tr -s ' \t\n' ' '; }
  if [ "$(git show $REV:$f | strip)" == "$(strip < $f)" ]; then echo "comments-only: $f"; else echo "CODE CHANGED: $f"; rc=1; fi
done; exit $rc
