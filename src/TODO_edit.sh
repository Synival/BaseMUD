#!/bin/sh
files=$(grep -l "TODO" *.c *.h)
# filtered=$(grep -L "PUT INTO TRELLO" $files)
vim trello.txt $files
