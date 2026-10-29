#!/bin/bash
# Run the whole game without a terminal on each tests/golden/*.keys and compare
# the transcript with the recorded *.transcript next to it.
#
# usage: tests/golden/run.sh [--update | --trace] [name...]
#   --update  write the transcripts as the new records
#   --trace   print the transcripts with the whole screen at every key
# A line "# run: <args>" starts a run of the game with those arguments (a seed
# with -w makes it repeatable); the keys after it, up to the next "# run:",
# are that run's script. All runs of one file share a working directory, so a
# later run can load what an earlier one saved. A line "# at end: eof" in a
# run's keys makes the input report end of input when they run out, instead
# of stopping the game. A line "# record: turns" adds the game turn to every
# key line of that run. Needs ./umoria_headless
# (make umoria_headless).
cd "$(dirname "$0")/../.." || exit 1
update=0; trace=
if [ "$1" = "--update" ]; then update=1; shift; fi
if [ "$1" = "--trace" ]; then trace=1; shift; fi
names=("$@")
if [ ${#names[@]} -eq 0 ]; then
    for k in tests/golden/*.keys; do names+=("$(basename "$k" .keys)"); done
fi
work=$(mktemp -d) || exit 1
trap 'rm -rf "$work"' EXIT
fail=0
for name in "${names[@]}"; do
    keys=tests/golden/$name.keys
    rm -rf "$work/run" "$work/parts"; mkdir "$work/run" "$work/parts"
    ln -s "$PWD/data" "$work/run/data"; : > "$work/run/scores.dat"
    # split into one key script per run
    awk -v dir="$work/parts" '/^# run: /{n++; sub(/^# run: /, ""); print > (dir "/" n ".args"); next} n {print > (dir "/" n ".keys")}' "$keys"
    : > "$work/$name.transcript"
    for a in $(ls "$work/parts"/*.args | sort -V); do
        part=${a%.args}
        [ -f "$part.keys" ] || : > "$part.keys"
        echo "run: $(cat "$a")" >> "$work/$name.transcript"
        at_end=$(sed -n 's/^# at end: //p' "$part.keys" | head -1)
        turns=$(grep -q '^# record: turns' "$part.keys" && echo 1)
        (cd "$work/run" && TZ=UTC HEADLESS_AT_END=$at_end HEADLESS_TURNS=$turns HEADLESS_TRACE=$trace HEADLESS_KEYS="$part.keys" HEADLESS_OUT="$part.transcript" \
            timeout 60 "$OLDPWD/umoria_headless" $(cat "$a") >/dev/null 2>&1)
        echo "exit status $?" >> "$work/$name.transcript"
        cat "$part.transcript" >> "$work/$name.transcript" 2>/dev/null
    done
    if [ -n "$trace" ]; then
        cat "$work/$name.transcript"
    elif [ $update -eq 1 ]; then
        cp "$work/$name.transcript" "tests/golden/$name.transcript"; echo "updated $name"
    elif cmp -s "$work/$name.transcript" "tests/golden/$name.transcript"; then
        echo "[ ok ] golden $name"
    else
        echo "[FAIL] golden $name"; diff "tests/golden/$name.transcript" "$work/$name.transcript" | head -20; fail=1
    fi
done
[ $update -eq 1 ] || [ -n "$trace" ] || { [ $fail -eq 0 ] && echo "golden: GREEN" || echo "golden: RED"; }
exit $fail
