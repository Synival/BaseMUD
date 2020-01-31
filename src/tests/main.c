#include <stdio.h>

extern void ext_flags_tests (void);

int main (void) {
    printf ("\x1b[2J\x1b[1;1H");
    fflush (stdout);

    ext_flags_tests ();
    return 0;
}
