#!/bin/bash
# Run the whole game without a terminal on each tests/golden/*.keys and compare
# the transcript with the recorded *.transcript next to it.
#
# usage: tests/golden/run.sh [--update] [name...]
# The first line "# args: ..." of a .keys file gives the game's arguments
# (a seed with -w makes the run repeatable). Needs ./umoria_headless (make umoria_headless).
cd "$(dirname "$0")/../.." || exit 1
update=0
if [ "$1" = "--update" ]; then update=1; shift; fi
names=("$@")
if [ ${#names[@]} -eq 0 ]; then
    for k in tests/golden/*.keys; do names+=("$(basename "$k" .keys)"); done
fi
work=$(mktemp -d) || exit 1
trap 'rm -rf "$work"' EXIT
fail=0
for name in "${names[@]}"; do
    keys=tests/golden/$name.keys
    args=$(sed -n '1s/^# args: //p' "$keys")
    rm -rf "$work/run"; mkdir "$work/run"; ln -s "$PWD/data" "$work/run/data"; : > "$work/run/scores.dat"
    (cd "$work/run" && HEADLESS_KEYS="$OLDPWD/$keys" HEADLESS_OUT="$work/$name.transcript" \
        timeout 60 "$OLDPWD/umoria_headless" $args game.sav >/dev/null 2>&1)
    if [ $update -eq 1 ]; then
        cp "$work/$name.transcript" "tests/golden/$name.transcript"; echo "updated $name"
    elif cmp -s "$work/$name.transcript" "tests/golden/$name.transcript"; then
        echo "[ ok ] golden $name"
    else
        echo "[FAIL] golden $name"; diff "tests/golden/$name.transcript" "$work/$name.transcript" | head -20; fail=1
    fi
done
[ $update -eq 1 ] || { [ $fail -eq 0 ] && echo "golden: GREEN" || echo "golden: RED"; }
exit $fail
