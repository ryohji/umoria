#!/bin/sh
# 本体ビルドのコンパイラ警告を数える。
#
# 警告は同じヘッダが翻訳単位ごとに読まれるため、そのまま数えると
# 同じ 1 行が何十回も出て実際の作業量が読めない。ここでは
# file:line:col を鍵にして重複を除き、警告フラグごとの箇所数を出す。
#
# 使い方:
#   scripts/warnings.sh            # フラグごとの箇所数
#   scripts/warnings.sh -Wshadow   # そのフラグの箇所を列挙
#
# 集計元は make のクリーンビルド。実行のたびに make clean する。

set -e
cd "$(dirname "$0")/.."

LOG=$(mktemp)
trap 'rm -f "$LOG"' EXIT

make clean >/dev/null 2>&1 || true
make 2>&1 | grep 'warning:' > "$LOG" || true

if [ $# -eq 0 ]; then
    printf '%s\n' "== 箇所数（file:line:col の重複を除いた数）=="
    sed -E 's/^([^ ]+:[0-9]+:[0-9]+): warning: .*(\[-W[a-z0-9=-]+\])$/\2 \1/' "$LOG" \
        | grep '^\[' | sort -u | awk '{print $1}' | uniq -c | sort -rn
    printf '%s\n' "-- フラグの付かない警告 --"
    grep -c -v '\[-W' "$LOG" || true
else
    grep -- "$1\]" "$LOG" | sort -u
fi
