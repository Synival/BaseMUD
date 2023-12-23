/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#include "fwrite.h"

#include "flags.h"
#include "ext_flags.h"
#include "save.h"

#include <string.h>

/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 *
 * *buf must hold at least 32+1 characters.
 *
 * -- Hugin
 */

char *fwrite_flags_static (const FLAG_T *table, flag_t flags) {
    static char buf[52];
    return fwrite_flags_buf (table, flags, buf);
}

char *fwrite_flags_buf (const FLAG_T *table, flag_t flags, char *buf) {
#ifndef BASEMUD_WRITE_FLAGS_OUT
    int count, pos = 0;
    for (count = 0; count < 32; count++) {
        if (IS_SET (flags, 1 << count)) {
            if (count < 26)
                buf[pos++] = 'A' + count;
            else
                buf[pos++] = 'a' + (count - 26);
        }
    }
    if (pos == 0)
        buf[pos++] = '0';
    buf[pos] = '\0';
#else
    sprintf (buf, "[%s]", flags_to_string_real (table, flags, ""));
#endif
    return buf;
}

char *fwrite_ext_flags_static (const EXT_FLAG_DEF_T *table, EXT_FLAGS_T flags) {
    static char buf[MAX_STRING_LENGTH];
    return fwrite_ext_flags_buf (table, flags, buf);
}

char *fwrite_ext_flags_buf (const EXT_FLAG_DEF_T *table, EXT_FLAGS_T flags,
    char *buf)
{
#ifndef BASEMUD_WRITE_EXT_FLAGS_OUT
    int count, pos = 0;
    for (count = 0; count < 32; count++) {
        if (EXT_IS_SET (flags, count)) {
            if (count < 26)
                buf[pos++] = 'A' + count;
            else
                buf[pos++] = 'a' + (count - 26);
        }
    }
    if (pos == 0)
        buf[pos++] = '0';
    buf[pos] = '\0';
#else
    sprintf (buf, "[%s]", ext_flags_to_string_real (table, flags, ""));
#endif
    return buf;
}
