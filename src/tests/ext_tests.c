#include <string.h>
#include <stdarg.h>

#include "../merc.h"

#include "utils.h"

#define EF_INDEX(_flag) ((flag) / 8)
#define EF_BIT(_flag)   (1 << ((flag) % 8))

EXT_FLAGS_T ext_flags_build (int first, ...) {
    EXT_FLAGS_T flags = EXT_ZERO;
    va_list ap;
    int flag;

    va_start (ap, first);
    flag = first;
    while (flag >= 0) {
        EXT_SET_BIT (flags, flag);
        flag = va_arg(ap, int);
    }
    va_end (ap);

    return flags;
}

EXT_FLAGS_T ext_flags_build_from_init (EXT_INIT_FLAGS_T *flags) {
    EXT_FLAGS_T new_flags = EXT_ZERO;
    int i;
    for (i = 0; flags->bits[i] >= 0; i++)
        EXT_SET_BIT (new_flags, flags->bits[i]);
    return new_flags;
}

bool ext_flags_is_set (EXT_FLAGS_T *var, int flag)
    { return (var->bits[EF_INDEX(flag)] & EF_BIT(flag)) ? TRUE : FALSE; }
void ext_flags_set_bit (EXT_FLAGS_T *var, int flag)
    { var->bits[EF_INDEX(flag)] |= EF_BIT(flag); }
void ext_flags_remove_bit (EXT_FLAGS_T *var, int flag)
    { var->bits[EF_INDEX(flag)] &= ~EF_BIT(flag); }
void ext_flags_toggle_bit (EXT_FLAGS_T *var, int flag)
    { var->bits[EF_INDEX(flag)] ^= EF_BIT(flag); }

static void print_flags(EXT_FLAGS_T *flags) {
    int index, bit;

    for (index = 0; index < EXT_FLAGS_ARRAY_LENGTH; index++) {
        print_position_column ();
        printf ("%2d: [", index);
        for (bit = 0; bit < EXT_FLAGS_ELEMENT_SIZE; bit++)
            printf ("%d", (flags->bits[index] & (1 << bit)) ? 1 : 0);
        printf ("]\n");
        print_next_line ();
    }
}

#define EB EXT_INIT_BITS
static EXT_INIT_FLAGS_T init_flags[EXT_FLAGS_LIMIT] = {
    EB( 0), EB( 1), EB( 2), EB( 3), EB( 4), EB( 5), EB( 6), EB( 7), EB( 8), EB( 9),
    EB(10), EB(11), EB(12), EB(13), EB(14), EB(15), EB(16), EB(17), EB(18), EB(19),
    EB(20), EB(21), EB(22), EB(23), EB(24), EB(25), EB(26), EB(27), EB(28), EB(29),
    EB(30), EB(31), EB(32), EB(33), EB(34), EB(35), EB(36), EB(37), EB(38), EB(39),
    EB(40), EB(41), EB(42), EB(43), EB(44), EB(45), EB(46), EB(47), EB(48), EB(49),
    EB(50), EB(51), EB(52), EB(53), EB(54), EB(55), EB(56), EB(57), EB(58), EB(59),
    EB(60), EB(61), EB(62), EB(63)
};

typedef struct {
    EXT_INIT_FLAGS_T init_flags;
    EXT_FLAGS_T expected_flags;
} EXT_INIT_EXPECTED_FLAGS_T;

static EXT_INIT_EXPECTED_FLAGS_T init_expected_flags[] = {
    { EB( 0,  1,  2,  3,  4,  5,  6,  7),         {{ 0xff, 0x00, 0x00, 0x00 }} },
    { EB( 8,  9, 10, 11, 12, 13, 14, 15),         {{ 0x00, 0xff, 0x00, 0x00 }} },
    { EB(16, 17, 18, 19, 20, 21, 22, 23),         {{ 0x00, 0x00, 0xff, 0x00 }} },
    { EB(24, 25, 26, 27, 28, 29, 30, 31),         {{ 0x00, 0x00, 0x00, 0xff }} },
    { EB(0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30), {{ 0x49, 0x92, 0x24, 0x49 }} },
      /* 0x01, 0x08, 0x40, 0x200, 0x1000, 0x8000, 0x40000, 0x200000, ... *
       * [9, 4: 0x49    ]  [2, 9: 0x92         ]  [4, 2: 0x24     ] [9, 4: 0x49] */
    { EXT_INIT_ZERO,                              {{ 0x00, 0x00, 0x00, 0x00 }} },
};

void ext_flags_tests (void) {
    EXT_FLAGS_T flags;
    int i, j;

    PRINT_LINE ("SETTING BITS:\n");
    PRINT_LINE ("\n");
    print_next_row();

    for (i = 0; i < EXT_FLAGS_LIMIT; i++) {
        print_position_row ();
        PRINT_LINE ("Bit: %d\n", i);

        if (EXT_IS_SET (flags, i))
            PRINT_LINE ("ALREADY SET!!\n");

        EXT_SET_BIT (flags, i);
        if (!EXT_IS_SET (flags, i))
            PRINT_LINE ("FLAG STILL NOT SET!!\n");

        EXT_REMOVE_BIT (flags, i);
        if (EXT_IS_SET (flags, i))
            PRINT_LINE ("STILL SET!!\n");

        EXT_TOGGLE_BIT (flags, i);
        if (!EXT_IS_SET (flags, i))
            PRINT_LINE ("FLAG NOT TOGGLED ON!!\n");

        EXT_TOGGLE_BIT (flags, i);
        if (EXT_IS_SET (flags, i))
            PRINT_LINE ("FLAG NOT TOGGLED OFF!!\n");

        print_next_col ();
    }
    PRINT_LINE ("\n");
    print_next_row ();

    PRINT_LINE ("INITIALIZING BITS:\n");
    PRINT_LINE ("\n");
    print_next_row ();

    for (i = 0; i < EXT_FLAGS_LIMIT; i++) {
        EXT_FLAGS_T flags = EXT_BITS_FROM_INIT (init_flags[i]);

        print_position_row ();
        PRINT_LINE ("Bit: %d\n", i);

        for (j = 0; j < EXT_FLAGS_LIMIT; j++) {
            int set = EXT_IS_SET (flags, j);
            if (set && i != j)
                PRINT_LINE ("WRONG BIT SET!\n");
            if (!set && i == j)
                PRINT_LINE ("CORRECT BIT NOT SET!\n");
        }

        print_next_col ();
    }
    PRINT_LINE ("\n");
    print_next_row ();

    PRINT_LINE ("INITIALIZING SPECIFIC BIT SETS:\n");
    PRINT_LINE ("\n");
    print_next_row ();

    for (i = 0; 1; i++) {
        EXT_INIT_EXPECTED_FLAGS_T *ief = &(init_expected_flags[i]);
        EXT_FLAGS_T flags = EXT_BITS_FROM_INIT (ief->init_flags);

        print_position_row ();
        PRINT_LINE ("Index: %d\n", i);

        for (j = 0; j < EXT_FLAGS_ARRAY_LENGTH; j++)
            if (flags.bits[j] != ief->expected_flags.bits[j])
                PRINT_LINE ("WRONG BITS @[%d]!\n", j);

        print_next_col ();
        if (ief->init_flags.bits[0] == -1)
            break;
    }
    PRINT_LINE ("\n");
    print_next_row ();

    PRINT_LINE ("BUILDING BITS AT COMPILE TIME:\n");
    PRINT_LINE ("\n");
    for (i = 0; i < EXT_FLAGS_LIMIT - 7; i++) {
        EXT_FLAGS_T flags = EXT_BITS (i, i + 4, i + 7);

        print_position_row ();
        PRINT_LINE ("Index: %d\n", i);

        for (j = 0; j < EXT_FLAGS_LIMIT; j++) {
            int is_set   = EXT_IS_SET (flags, j);
            int expected = (j == i || j == i + 4 || j == i + 7);
            if (is_set && !expected)
                PRINT_LINE ("WRONG BIT SET!\n");
            if (!is_set && expected)
                PRINT_LINE ("CORRECT BIT NOT SET!\n");
        }

        print_next_col ();
    }
    PRINT_LINE ("\n");
    print_next_row ();
}
