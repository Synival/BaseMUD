#!/bin/sh
count=$(grep "TODO" *.c *.h | wc -l)
files=$(grep -Hl "TODO" *.c *.h | wc -l)

echo "${count} TODOs in ${files} files."
