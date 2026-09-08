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

BUILD=$(mktemp)
WARN=$(mktemp)
trap 'rm -f "$BUILD" "$WARN"' EXIT

# ビルドが途中で失敗すると、それ以降のファイルはコンパイルされないので
# 警告の数だけ見ていると「減った」と読みちがえる。まず成否を判定する。
make clean >/dev/null 2>&1 || true
if ! make > "$BUILD" 2>&1; then
    printf '%s\n' "== ビルド失敗（警告の集計は当てにならない）=="
    grep -B3 'error:' "$BUILD" || tail -20 "$BUILD"
    exit 1
fi

grep 'warning:' "$BUILD" > "$WARN" || true

if [ $# -eq 0 ]; then
    printf '%s\n' "== 箇所数（file:line:col の重複を除いた数）=="
    sed -E 's/^([^ ]+:[0-9]+:[0-9]+): warning: .*(\[-W[a-z0-9=-]+\])$/\2 \1/' "$WARN" \
        | grep '^\[' | sort -u | awk '{print $1}' | uniq -c | sort -rn
    printf '%s\n' "-- フラグの付かない警告（箇所数）--"
    grep -v '\[-W' "$WARN" | sort -u | wc -l
else
    grep -- "$1\]" "$WARN" | sort -u
fi
