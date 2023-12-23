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

#include "help.h"

#include <stdio.h>

void help_area_to_area (HELP_AREA_T *had, AREA_T *area) {
    LIST2_REASSIGN_BACK (had, area, area_prev, area_next,
        area, had_first, had_last);
}

void help_to_help_area (HELP_T *help, HELP_AREA_T *had) {
    LIST2_REASSIGN_BACK (help, had, had_prev, had_next,
        had, help_first, help_last);
}

int help_area_count_pages (HELP_AREA_T *had) {
    HELP_T *h;
    int count = 0;

    for (h = had->help_first; h != NULL; h = h->had_next)
        count++;
    return count;
}
