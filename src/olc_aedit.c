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

#include "olc_aedit.h"

#include "areas.h"
#include "comm.h"
#include "db.h"
#include "flags.h"
#include "globals.h"
#include "interp.h"
#include "lookup.h"
#include "memory.h"
#include "olc.h"
#include "recycle.h"
#include "string.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*****************************************************************************
 Name:       aedit_check_range( lower vnum, upper vnum )
 Purpose:    Ensures the range spans only one area.
 Called by:  aedit_vnum(olc_act.c).
 ****************************************************************************/
bool aedit_check_range (int lower, int upper) {
    AREA_T *area;
    int cnt = 0;

    for (area = area_first; area; area = area->global_next) {
        /* lower < area < upper */
        if ((lower <= area->min_vnum && area->min_vnum <= upper)
            || (lower <= area->max_vnum && area->max_vnum <= upper))
            ++cnt;
        if (cnt > 1)
            return FALSE;
    }
    return TRUE;
}

AEDIT (aedit_show) {
    AREA_T *area;
    EDIT_AREA (ch, area);

    printf_to_char (ch, "Name:     [%5d] %s\n\r", area->vnum, area->name);
    printf_to_char (ch, "File:     %s\n\r", area->filename);
    printf_to_char (ch, "Title:    %s\n\r", area->title);

#if 0                            /* ROM OLC */
    printf_to_char (ch, "Recall:   [%5d] %s\n\r", area->recall,
        room_get_index (area->recall) ? room_get_index (area->recall)->name
                                       : "none");
#endif /* ROM */

    printf_to_char (ch, "Vnums:    [%d-%d]\n\r",
        area->min_vnum, area->max_vnum);
    printf_to_char (ch, "Age:      [%d]\n\r", area->age);
    printf_to_char (ch, "Players:  [%d]\n\r", area->nplayer);
    printf_to_char (ch, "Security: [%d]\n\r", area->security);
    printf_to_char (ch, "Builders: [%s]\n\r", area->builders);
    printf_to_char (ch, "Credits : [%s]\n\r", area->credits);
    printf_to_char (ch, "Flags:    [%s]\n\r",
        flags_to_string (area_flags, area->area_flags));

    return FALSE;
}

AEDIT (aedit_reset) {
    AREA_T *area;
    EDIT_AREA (ch, area);

    area_reset (area);
    send_to_char ("Area reset.\n\r", ch);
    return FALSE;
}

AEDIT (aedit_create) {
    AREA_T *area;

    area = area_new ();
    LIST2_BACK (area, global_prev, global_next, area_first, area_last);
    ch->desc->olc_edit = (void *) area;

    SET_BIT (area->area_flags, AREA_ADDED);
    send_to_char ("Area created.\n\r", ch);
    return FALSE;
}

AEDIT (aedit_title) {
    AREA_T *area;
    EDIT_AREA (ch, area);
    return olc_str_replace_dup (ch, &(area->title), argument,
        "Syntax: name [$name]\n\r",
        "Title set.\n\r");
}

AEDIT (aedit_credits) {
    AREA_T *area;
    EDIT_AREA (ch, area);
    return olc_str_replace_dup (ch, &(area->credits), argument,
        "Syntax: credits [$credits]\n\r",
        "Credits set.\n\r");
}

AEDIT (aedit_file) {
    AREA_T *area;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA (ch, area);
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

    str_replace_dup (&(area->name), file);
    strcat (file, ".are");
    str_replace_dup (&(area->filename), file);
    send_to_char ("Filename set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_age) {
    AREA_T *area;
    EDIT_AREA (ch, area);
    return olc_sh_int_replace (ch, &(area->age), argument,
        "Syntax: age [#xage]\n\r", "Age set.\n\r");
}

#if 0 /* ROM OLC */
AEDIT (aedit_recall) {
    AREA_T *area;
    char room[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA (ch, area);
    one_argument (argument, room);

    if (!is_number (argument) || argument[0] == '\0') {
        send_to_char ("Syntax:  recall [#xrvnum]\n\r", ch);
        return FALSE;
    }

    value = atoi (room);
    if (!room_get_index (value)) {
        send_to_char ("AEdit:  Room vnum does not exist.\n\r", ch);
        return FALSE;
    }

    area->recall = value;
    send_to_char ("Recall set.\n\r", ch);
    return TRUE;
}
#endif /* ROM OLC */

AEDIT (aedit_security) {
    AREA_T *area;
    char sec[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA (ch, area);
    one_argument (argument, sec);

    RETURN_IF (!is_number (sec) || sec[0] == '\0',
        "Syntax: security [#xlevel]\n\r", ch, FALSE);

    value = atoi (sec);
    if (value > ch->pcdata->security || value < 0) {
        if (ch->pcdata->security != 0)
            printf_to_char (ch, "Security is 0-%d.\n\r", ch->pcdata->security);
        else
            send_to_char ("Security is 0 only.\n\r", ch);
        return FALSE;
    }

    area->security = value;
    send_to_char ("Security set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_builder) {
    AREA_T *area;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA (ch, area);
    one_argument (argument, name);

    RETURN_IF (name[0] == '\0',
        "Syntax: builder [$name]  -toggles builder\n\r"
        "Syntax: builder All      -allows everyone\n\r", ch, FALSE);

    name[0] = UPPER (name[0]);
    if (strstr (area->builders, name) != NULL) {
        area->builders = string_replace (area->builders, name, "\0");
        area->builders = string_unpad (area->builders);

        if (area->builders[0] == '\0')
            str_replace_dup (&(area->builders), "None");
        send_to_char ("Builder removed.\n\r", ch);
        return TRUE;
    }
    else {
        buf[0] = '\0';
        if (strstr (area->builders, "None") != NULL) {
            area->builders = string_replace (area->builders, "None", "\0");
            area->builders = string_unpad (area->builders);
        }
        if (area->builders[0] != '\0') {
            strcat (buf, area->builders);
            strcat (buf, " ");
        }

        strcat (buf, name);
        str_free (&(area->builders));
        area->builders = string_proper (str_dup (buf));

        printf_to_char (ch, "Builder added.\n\r"
                            "%s\n\r", area->builders);
        return TRUE;
    }
    return FALSE;
}

AEDIT (aedit_vnum) {
    AREA_T *area, *other;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, area);

    argument = one_argument (argument, lower);
    one_argument (argument, upper);

    RETURN_IF (!is_number (lower) || lower[0] == '\0' ||
               !is_number (upper) || upper[0] == '\0',
        "Syntax: vnum [#xlower] [#xupper]\n\r", ch, FALSE);

    RETURN_IF ((ilower = atoi (lower)) > (iupper = atoi (upper)),
        "AEdit: Upper must be larger then lower.\n\r", ch, FALSE);
    RETURN_IF (!aedit_check_range (atoi (lower), atoi (upper)),
        "AEdit: Range must include only this area.\n\r", ch, FALSE);

    other = area_get_by_inner_vnum (ilower);
    RETURN_IF (other && other != area,
        "AEdit: Lower vnum already assigned.\n\r", ch, FALSE);

    area->min_vnum = ilower;
    send_to_char ("Lower vnum set.\n\r", ch);

    other = area_get_by_inner_vnum (iupper);
    RETURN_IF (other && other != area,
        "AEdit: Upper vnum already assigned.\n\r", ch, TRUE);
                        /* The lower value has been set ^^^^ */

    area->max_vnum = iupper;
    send_to_char ("Upper vnum set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_lvnum) {
    AREA_T *area, *other;
    char lower[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, area);

    one_argument (argument, lower);

    RETURN_IF (!is_number (lower) || lower[0] == '\0',
        "Syntax: min_vnum [#xlower]\n\r", ch, FALSE);
    RETURN_IF ((ilower = atoi (lower)) > (iupper = area->max_vnum),
        "AEdit: Value must be less than the max_vnum.\n\r", ch, FALSE);
    RETURN_IF (!aedit_check_range (ilower, iupper),
        "AEdit: Range must include only this area.\n\r", ch, FALSE);

    other = area_get_by_inner_vnum (ilower);
    RETURN_IF (other && other != area,
        "AEdit: Lower vnum already assigned.\n\r", ch, FALSE);

    area->min_vnum = ilower;
    send_to_char ("Lower vnum set.\n\r", ch);
    return TRUE;
}

AEDIT (aedit_uvnum) {
    AREA_T *area, *other;
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA (ch, area);

    one_argument (argument, upper);
    RETURN_IF (!is_number (upper) || upper[0] == '\0',
        "Syntax: max_vnum [#xupper]\n\r", ch, FALSE);
    RETURN_IF ((ilower = area->min_vnum) > (iupper = atoi (upper)),
        "AEdit: Upper must be larger then lower.\n\r", ch, FALSE);
    RETURN_IF (!aedit_check_range (ilower, iupper),
        "AEdit: Range must include only this area.\n\r", ch, FALSE);

    other = area_get_by_inner_vnum (iupper);
    RETURN_IF (other && other != area,
        "AEdit: Upper vnum already assigned.\n\r", ch, FALSE);

    area->max_vnum = iupper;
    send_to_char ("Upper vnum set.\n\r", ch);
    return TRUE;
}
