/***************************************************************************
 *  File: olc_hedit.c                                                      *
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
#include "lookup.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "string.h"
#include "olc.h"

#include "olc_hedit.h"

HEDIT (hedit_show) {
    HELP_DATA *help;
    EDIT_HELP (ch, help);

    printf_to_char (ch,
        "Keyword: [%s]\n\r"
        "Level  : [%d]\n\r"
        "Text   :\n\r"
        "%s-END-\n\r", help->keyword, help->level, help->text);
    return FALSE;
}

HEDIT (hedit_level) {
    HELP_DATA *help;
    int lev;

    EDIT_HELP (ch, help);
    RETURN_IF (IS_NULLSTR (argument) || !is_number (argument),
        "Syntax: level [-1..MAX_LEVEL]\n\r", ch, FALSE);

    lev = atoi (argument);
    if (lev < -1 || lev > MAX_LEVEL) {
        printf_to_char (ch,
            "HEdit: levels are between -1 and %d inclusive.\n\r", MAX_LEVEL);
        return FALSE;
    }

    help->level = lev;
    send_to_char ("Ok.\n\r", ch);
    return TRUE;
}

HEDIT (hedit_keyword) {
    HELP_DATA *help;
    EDIT_HELP (ch, help);

    RETURN_IF (IS_NULLSTR (argument),
        "Syntax: keyword [keywords]\n\r", ch, FALSE);

    str_replace_dup (&(help->keyword), argument);
    send_to_char ("Ok.\n\r", ch);
    return TRUE;
}

HEDIT (hedit_new) {
    char arg[MIL], fullarg[MIL];
    HELP_AREA *had;
    HELP_DATA *help;
    extern HELP_DATA *help_last;

    RETURN_IF (IS_NULLSTR (argument),
        "Syntax: new [name]\n\r"
        "        new [area] [name]\n\r", ch, FALSE);

    strcpy (fullarg, argument);
    argument = one_argument (argument, arg);

    if (!(had = had_get_by_name (arg))) {
        had = ch->in_room->area->helps;
        argument = fullarg;
    }
    RETURN_IF (help_get_by_name (argument),
        "HEdit: Help already exists.\n\r", ch, FALSE);

    /* the area has no helps */
    if (!had) {
        had = had_new ();
        str_replace_dup (&had->filename, ch->in_room->area->filename);
        str_replace_dup (&had->name, trim_extension (had->filename));
        had->area = ch->in_room->area;
        had->changed = TRUE;
        LISTB_BACK (had, next, had_first, had_last);
        ch->in_room->area->helps = had;
        SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
    }

    help = help_new ();
    help->level = 0;
    help->keyword = str_dup (argument);
    help->text = str_dup ("");

    LISTB_BACK (help, next, help_first, help_last);
    LISTB_BACK (help, next_area, had->first, had->last);

    ch->desc->pEdit = (HELP_DATA *) help;
    ch->desc->editor = ED_HELP;

    send_to_char ("Help created.\n\r", ch);
    return FALSE;
}

HEDIT (hedit_text) {
    HELP_DATA *help;
    EDIT_HELP (ch, help);

    RETURN_IF (!IS_NULLSTR (argument),
        "Syntax: text\n\r", ch, FALSE);

    string_append (ch, &help->text);
    return TRUE;
}

HEDIT (hedit_delete) {
    HELP_DATA *pHelp, *hlp, *phlp;
    HELP_AREA *had;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    EDIT_HELP (ch, pHelp);
    for (d = descriptor_list; d; d = d->next)
        if (d->editor == ED_HELP && pHelp == (HELP_DATA *) d->pEdit)
            edit_done (d->character);

    /* Remove help from the global help list. Bail if this fails. */
    LISTB_REMOVE (pHelp, next, help_first, help_last, HELP_DATA, return FALSE);

    /* Find a help area with this help entry. If found, remove it. */
    for (had = had_first; had; had = had->next) {
        LIST_FIND_WITH_PREV (hlp == pHelp, next_area, had->first, hlp, phlp);
        if (hlp) {
            LISTB_REMOVE_WITH_PREV (hlp, phlp, next_area,
                had->first, had->last);
            found = TRUE;
            break;
        }
    }
    RETURN_IF_BUGF (!found, FALSE,
        "HEdit delete: Help %s not found in had_list.", pHelp->keyword);
    help_free (pHelp);

    send_to_char ("Help deleted.\n\r", ch);
    return TRUE;
}

HEDIT (hedit_list) {
    char buf[MIL];
    int cnt = 0;
    HELP_DATA *pHelp;
    BUFFER *buffer;

    EDIT_HELP (ch, pHelp);

    if (!str_cmp (argument, "all")) {
        buffer = buf_new ();
        for (pHelp = help_first; pHelp; pHelp = pHelp->next) {
            sprintf (buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
                     cnt % 4 == 3 ? "\n\r" : " ");
            add_buf (buffer, buf);
            cnt++;
        }
        if (cnt % 4)
            add_buf (buffer, "\n\r");

        page_to_char (buf_string (buffer), ch);
        return FALSE;
    }

    if (!str_cmp (argument, "area")) {
        RETURN_IF (ch->in_room->area->helps == NULL,
            "No helps in this area.\n\r", ch, FALSE);

        buffer = buf_new ();
        for (pHelp = ch->in_room->area->helps->first; pHelp;
             pHelp = pHelp->next_area)
        {
            sprintf (buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
                     cnt % 4 == 3 ? "\n\r" : " ");
            add_buf (buffer, buf);
            cnt++;
        }

        if (cnt % 4)
            add_buf (buffer, "\n\r");

        page_to_char (buf_string (buffer), ch);
        return FALSE;
    }

    RETURN_IF (IS_NULLSTR (argument),
        "Syntax: list all\n\r"
        "        list area\n\r", ch, FALSE);

    return FALSE;
}
