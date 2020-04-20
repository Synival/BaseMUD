/***************************************************************************
 *  File: olc_mpedit.c                                                     *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "comm.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "string.h"
#include "lookup.h"
#include "chars.h"
#include "globals.h"
#include "olc.h"
#include "memory.h"
#include "mob_prog.h"

#include "olc_mpedit.h"

MPEDIT (mpedit_create) {
    MPROG_CODE_T *mcode;
    int value = atoi (argument);
    AREA_T *ad;

    RETURN_IF (IS_NULLSTR (argument) || value < 1,
        "Syntax: mpedit create [vnum]\n\r", ch, FALSE);

    ad = area_get_by_inner_vnum (value);
    RETURN_IF (ad == NULL,
        "MPEdit: That area vnum does not exist.\n\r", ch, FALSE);
    RETURN_IF (!IS_BUILDER (ch, ad),
        "MPEdit: Vnum in an area you cannot build in.\n\r", ch, FALSE);
    RETURN_IF (mpcode_get_index (value),
        "MPEdit: Mob program vnum already exists.\n\r", ch, FALSE);

    mcode = mpcode_new ();
    mpcode_to_area (mcode, ad);
    mcode->vnum = value;
    mcode->anum = value - ad->min_vnum;
    LIST2_FRONT (mcode, global_prev, global_next, mpcode_first, mpcode_last);
    ch->desc->olc_edit = (void *) mcode;
    ch->desc->editor = ED_MPCODE;

    printf_to_char (ch, "Mob program created.\n\r");
    return TRUE;
}

MPEDIT (mpedit_show) {
    MPROG_CODE_T *mcode;
    EDIT_MPCODE (ch, mcode);

    printf_to_char (ch,
         "Vnum:       [%d]\n\r"
         "Code:\n\r%s\n\r", mcode->vnum, mcode->code);
    return FALSE;
}

MPEDIT (mpedit_code) {
    MPROG_CODE_T *mcode;
    EDIT_MPCODE (ch, mcode);

    if (argument[0] == '\0') {
        string_append (ch, &mcode->code);
        return TRUE;
    }

    printf_to_char (ch, "Syntax: code\n\r");
    return FALSE;
}

MPEDIT (mpedit_list) {
    int count = 1;
    MPROG_CODE_T *mprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER_T *buffer;
    bool all = !str_cmp (argument, "all");
    char chr;
    AREA_T *ad;

    buffer = buf_new ();
    for (mprg = mpcode_first; mprg != NULL; mprg = mprg->global_next) {
        if (all || ENTRE (
            ch->in_room->area->min_vnum, mprg->vnum,
            ch->in_room->area->max_vnum))
        {
            ad = area_get_by_inner_vnum (mprg->vnum);
            if (ad == NULL)
                chr = '?';
            else if (IS_BUILDER (ch, ad))
                chr = '*';
            else
                chr = ' ';

            sprintf (buf, "[%3d] (%c) %5d\n\r", count, chr, mprg->vnum);
            buf_cat (buffer, buf);
            count++;
        }
    }

    if (count == 1) {
        if (all)
            buf_cat (buffer, "MobPrograms do not exist!\n\r");
        else
            buf_cat (buffer, "MobPrograms do not exist in this area.\n\r");
    }

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
    return FALSE;
}
