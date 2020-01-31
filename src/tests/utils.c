#include <stdio.h>

#include "utils.h"

static int pos = 0, line = 0;

void print_next_row(void) {
    pos  = 0;
    line = 0;
}

void print_next_col(void) {
    pos++;
    if (pos >= 8)
        print_next_row ();
}

void print_position_row(void) {
    if (pos > 0 && line > 0)
        printf ("\x1b[%dA", line);
    line = 0;
}

void print_position_column(void) {
    int col = pos * 16;
    printf ("\x1b[%dC", col + 1);
}

void print_next_line(void) {
    line++;
}
