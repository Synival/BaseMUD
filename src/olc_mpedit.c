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
#include "olc.h"
#include "chars.h"

#include "olc_mpedit.h"

MPEDIT (mpedit_create) {
    MPROG_CODE *pMcode;
    int value = atoi (argument);
    AREA_DATA *ad;

    RETURN_IF (IS_NULLSTR (argument) || value < 1,
        "Syntax: mpedit create [vnum]\n\r", ch, FALSE);

    ad = area_get_by_inner_vnum (value);
    RETURN_IF (ad == NULL,
        "MPEdit: That area vnum does not exist.\n\r", ch, FALSE);
    RETURN_IF (!IS_BUILDER (ch, ad),
        "MPEdit: Vnum in an area you cannot build in.\n\r", ch, FALSE);
    RETURN_IF (get_mprog_index (value),
        "MPEdit: Mob program vnum already exists.\n\r", ch, FALSE);

    pMcode = mpcode_new ();
    pMcode->area = ad;
    pMcode->vnum = value;
    pMcode->anum = value - ad->min_vnum;
    LIST_FRONT (pMcode, next, mprog_list);
    ch->desc->pEdit = (void *) pMcode;
    ch->desc->editor = ED_MPCODE;

    send_to_char ("Mob program created.\n\r", ch);
    return TRUE;
}

MPEDIT (mpedit_show) {
    MPROG_CODE *pMcode;
    EDIT_MPCODE (ch, pMcode);

    printf_to_char (ch,
         "Vnum:       [%d]\n\r"
         "Code:\n\r%s\n\r", pMcode->vnum, pMcode->code);
    return FALSE;
}

MPEDIT (mpedit_code) {
    MPROG_CODE *pMcode;
    EDIT_MPCODE (ch, pMcode);

    if (argument[0] == '\0') {
        string_append (ch, &pMcode->code);
        return TRUE;
    }

    send_to_char ("Syntax: code\n\r", ch);
    return FALSE;
}

MPEDIT (mpedit_list) {
    int count = 1;
    MPROG_CODE *mprg;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    bool fAll = !str_cmp (argument, "all");
    char chr;
    AREA_DATA *ad;

    buffer = buf_new ();
    for (mprg = mprog_list; mprg != NULL; mprg = mprg->next) {
        if (fAll
            || ENTRE (ch->in_room->area->min_vnum, mprg->vnum,
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
            add_buf (buffer, buf);
            count++;
        }
    }

    if (count == 1) {
        if (fAll)
            add_buf (buffer, "MobPrograms do not exist!\n\r");
        else
            add_buf (buffer, "MobPrograms do not exist in this area.\n\r");
    }

    page_to_char (buf_string (buffer), ch);
    buf_free (buffer);
    return FALSE;
}
