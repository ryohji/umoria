#!/bin/bash
# Data objects of two executables: the multiset of (type, size) must match.
# usage: scripts/check_statics.sh OLD_BINARY NEW_BINARY
f(){ nm -S "$1" | awk 'NF==4 && $3 ~ /^[bBdDrR]$/ && $4 !~ /^(__|\.|completed|CSWTCH|_)/ {print tolower($3), $2}' | sort | uniq -c; }
if diff <(f "$1") <(f "$2"); then echo "OK: data objects (type,size) match: $(f "$1" | awk '{s+=$1} END{print s}')"; else echo "DATA OBJECTS DIFFER"; exit 1; fi
