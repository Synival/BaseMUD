#!/bin/sh
files=$(grep -l "TODO" *.c *.h)
vim TODO.txt $files
