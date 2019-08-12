/***************************************************************************
 *  File: olc_aedit.c                                                      *
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
#include <ctype.h>

#include "comm.h"
#include "lookup.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "string.h"
#include "olc.h"

#include "olc_aedit.h"

/*****************************************************************************
 Name:        check_range( lower vnum, upper vnum )
 Purpose:    Ensures the range spans only one area.
 Called by:    aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range (int lower, int upper) {
    AREA_DATA *pArea;
    int cnt = 0;

    for (pArea = area_first; pArea; pArea = pArea->next) {
        /* lower < area < upper */
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper)
            || (lower <= pArea->max_vnum && pArea->max_vnum <= upper))
            ++cnt;
        if (cnt > 1)
            return FALSE;
    }
    return TRUE;
}

AEDIT (aedit_show) {
    AREA_DATA *pArea;
    EDIT_AREA (ch, pArea);

    printf_to_char (ch, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name);
    printf_to_char (ch, "File:     %s\n\r", pArea->filename);
    printf_to_char (ch, "Title:    %s\n\r", pArea->title);

#if 0                            /* ROM OLC */
    printf_to_char (ch, "Recall:   [%5d] %s\n\r", pArea->recall,
        get_room_index (pArea->recall) ? get_room_index (pArea->recall)->name
                                       : "none");
#endif /* ROM */

    printf_to_char (ch, "Vnums:    [%d-%d]\n\r",
        pArea->min_vnum, pArea->max_vnum);
    printf_to_char (ch, "Age:      [%d]\n\r", pArea->age);
    printf_to_char (ch, "Players:  [%d]\n\r", pArea->nplayer);
    printf_to_char (ch, "Security: [%d]\n\r", pArea->security);
    printf_to_char (ch, "Builders: [%s]\n\r", pArea->builders);
    printf_to_char (ch, "Credits : [%s]\n\r", pArea->credits);
    printf_to_char (ch, "Flags:    [%s]\n\r",
        flag_string (area_flags, pArea->area_flags));

    return FALSE;
}

AEDIT (aedit_reset) {
    AREA_DATA *pArea;
    EDIT_AREA (ch, pArea);

    reset_area (pArea);
    send_to_char ("Area reset.\n\r", ch);
    return FALSE;
}

AEDIT (aedit_create) {
    AREA_DATA *pArea;

    pArea = area_new ();
    LISTB_BACK (pArea, next, area_first, area_last);
    ch->desc->pEdit = (void *) pArea;

    SET_BIT (pArea->area_flags, AREA_ADDED);
    send_to_char ("Area Created.\n\r", ch);
    return FALSE;
}

AEDIT (aedit_title) {
    AREA_DATA *pArea;
    EDIT_AREA (ch, pArea);
    return olc_str_replace_dup (ch, &(pArea->title), argument,
        "Syntax:   name [$name]\n\r",
        "Title set.\n\r");
}

AEDIT (aedit_credits) {
    AREA_DATA *pArea;
    EDIT_AREA (ch, pArea);
    return olc_str_replace_dup (ch, &(pArea->credits), argument,
        "Syntax:   credits [$credits]\n\r",
        "Credits set.\n\r");
}

AEDIT (aedit_file) {
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA (ch, pArea);
    one_argument (argument, file); /* Forces Lowercase */

    RETURN_IF (argument[0] == '\0',
        "Syntax: filename [$file]\n\r", ch, FALSE);

    /* Simple Syntax Check. */
    length = strlen (argument);
    RETURN_IF (length > 8,
        "No more than eight characters allowed.\n\r", ch, FALSE);

    /* Allow only letters and numbers. */
    for (i = 0; i < length; i++)
        RETURN_IF (!isalnum (file[i]),
            "Only letters and numbers are valid.\n\r", ch, FALSE);

    strcat (file, ".are");
    str_replace_dup (&(pArea->filename), file);
    send_to_char ("Filename set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_age) {
    AREA_DATA *pArea;
    EDIT_AREA (ch, pArea);
    return olc_sh_int_replace (ch, &(pArea->age), argument,
        "Syntax:  age [#xage]\n\r", "Age set.\n\r");
}

#if 0 /* ROM OLC */
AEDIT (aedit_recall) {
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA (ch, pArea);
    one_argument (argument, room);

    if (!is_number (argument) || argument[0] == '\0') {
        send_to_char ("Syntax:  recall [#xrvnum]\n\r", ch);
        return FALSE;
    }

    value = atoi (room);
    if (!get_room_index (value)) {
        send_to_char ("AEdit:  Room vnum does not exist.\n\r", ch);
        return FALSE;
    }

    pArea->recall = value;
    send_to_char ("Recall set.\n\r", ch);
    return TRUE;
}
#endif /* ROM OLC */

AEDIT (aedit_security) {
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA (ch, pArea);
    one_argument (argument, sec);

    RETURN_IF (!is_number (sec) || sec[0] == '\0',
        "Syntax:  security [#xlevel]\n\r", ch, FALSE);

    value = atoi (sec);
    if (value > ch->pcdata->security || value < 0) {
        if (ch->pcdata->security != 0)
            printf_to_char (ch, "Security is 0-%d.\n\r", ch->pcdata->security);
        else
            send_to_char ("Security is 0 only.\n\r", ch);
        return FALSE;
    }

    pArea->security = value;
    send_to_char ("Security set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_builder) {
    AREA_DATA *pArea;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA (ch, pArea);
    one_argument (argument, name);

    RETURN_IF (name[0] == '\0',
        "Syntax:  builder [$name]  -toggles builder\n\r"
        "Syntax:  builder All      -allows everyone\n\r", ch, FALSE);

    name[0] = UPPER (name[0]);
    if (strstr (pArea->builders, name) != '\0') {
        pArea->builders = string_replace (pArea->builders, name, "\0");
        pArea->builders = string_unpad (pArea->builders);

        if (pArea->builders[0] == '\0')
            str_replace_dup (&(pArea->builders), "None");
        send_to_char ("Builder removed.\n\r", ch);
        return TRUE;
    }
    else {
        buf[0] = '\0';
        if (strstr (pArea->builders, "None") != '\0') {
            pArea->builders = string_replace (pArea->builders, "None", "\0");
            pArea->builders = string_unpad (pArea->builders);
        }
        if (pArea->builders[0] != '\0') {
            strcat (buf, pArea->builders);
            strcat (buf, " ");
        }

        strcat (buf, name);
        str_free (pArea->builders);
        pArea->builders = string_proper (str_dup (buf));

        send_to_char ("Builder added.\n\r", ch);
        send_to_char (pArea->builders, ch);
        return TRUE;
    }
    return FALSE;
}

AEDIT (aedit_vnum) {
    AREA_DATA *pArea, *other;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, pArea);

    argument = one_argument (argument, lower);
    one_argument (argument, upper);

    RETURN_IF (!is_number (lower) || lower[0] == '\0' ||
               !is_number (upper) || upper[0] == '\0',
        "Syntax:  vnum [#xlower] [#xupper]\n\r", ch, FALSE);

    RETURN_IF ((ilower = atoi (lower)) > (iupper = atoi (upper)),
        "AEdit:  Upper must be larger then lower.\n\r", ch, FALSE);
    RETURN_IF (!check_range (atoi (lower), atoi (upper)),
        "AEdit:  Range must include only this area.\n\r", ch, FALSE);

    other = area_get_by_inner_vnum (ilower);
    RETURN_IF (other && other != pArea,
        "AEdit:  Lower vnum already assigned.\n\r", ch, FALSE);

    pArea->min_vnum = ilower;
    send_to_char ("Lower vnum set.\n\r", ch);

    other = area_get_by_inner_vnum (iupper);
    RETURN_IF (other && other != pArea,
        "AEdit:  Upper vnum already assigned.\n\r", ch, TRUE);
                        /* The lower value has been set ^^^^ */

    pArea->max_vnum = iupper;
    send_to_char ("Upper vnum set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_lvnum) {
    AREA_DATA *pArea, *other;
    char lower[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, pArea);

    one_argument (argument, lower);

    RETURN_IF (!is_number (lower) || lower[0] == '\0',
        "Syntax:  min_vnum [#xlower]\n\r", ch, FALSE);
    RETURN_IF ((ilower = atoi (lower)) > (iupper = pArea->max_vnum),
        "AEdit:  Value must be less than the max_vnum.\n\r", ch, FALSE);
    RETURN_IF (!check_range (ilower, iupper),
        "AEdit:  Range must include only this area.\n\r", ch, FALSE);

    other = area_get_by_inner_vnum (ilower);
    RETURN_IF (other && other != pArea,
        "AEdit:  Lower vnum already assigned.\n\r", ch, FALSE);

    pArea->min_vnum = ilower;
    send_to_char ("Lower vnum set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_uvnum) {
    AREA_DATA *pArea, *other;
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, pArea);

    one_argument (argument, upper);
    RETURN_IF (!is_number (upper) || upper[0] == '\0',
        "Syntax:  max_vnum [#xupper]\n\r", ch, FALSE);
    RETURN_IF ((ilower = pArea->min_vnum) > (iupper = atoi (upper)),
        "AEdit:  Upper must be larger then lower.\n\r", ch, FALSE);
    RETURN_IF (!check_range (ilower, iupper),
        "AEdit:  Range must include only this area.\n\r", ch, FALSE);

    other = area_get_by_inner_vnum (iupper);
    RETURN_IF (other && other != pArea,
        "AEdit:  Upper vnum already assigned.\n\r", ch, FALSE);

    pArea->max_vnum = iupper;
    send_to_char ("Upper vnum set.\n\r", ch);
    return TRUE;
}
