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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

/* NOTE
 * ----
 * This file contains sub-routines for do_*() commands that can be shared
 * amongst themselves, as well as "filter" functions that perform a task
 * along with controlling code flow. There's a lot of miscellaneous stuff in
 * here that could probably be split into separate files if necessary,
 * but the overall goal here is to make commands shorter. If you notice
 * duplicate code between functions that can be extracted, do it here! If
 * you see code that belongs only to one function, but could be split into
 * separate functions (like ostat, mstat, rstat), split them into separate
 * do_() commands but leave them in their respective files.
 *    --- Synival */

#include "comm.h"
#include "chars.h"

#include "do_sub.h"

void do_autolist_flag (char *name, CHAR_DATA * ch, flag_t flags, flag_t flag) {
    int padding = 15, i;
    char *msg = IS_SET (flags, flag) ? "{GON{x" : "{ROFF{x";

    for (i = 0; name[i] != '\0'; i++)
        if (name[i] == '{')
            padding += 2;
    printf_to_char (ch, "   %-*s%s\n\r", padding, name, msg);
}

void do_flag_toggle (CHAR_DATA * ch, int player_only, flag_t *flags,
    flag_t flag, char *off_msg, char *on_msg)
{
    BAIL_IF (player_only && IS_NPC (ch),
        "NPCs can't use player flags.\n\r", ch);

    if (IS_SET ((*flags), flag)) {
        send_to_char (off_msg, ch);
        REMOVE_BIT ((*flags), flag);
    }
    else {
        send_to_char (on_msg, ch);
        SET_BIT ((*flags), flag);
    }
}

bool do_comm_toggle_channel_if_blank (CHAR_DATA *ch, char *argument,
    flag_t channel, char *message_on, char *message_off)
{
    if (argument[0] != '\0')
        return FALSE;
    do_flag_toggle (ch, FALSE, &(ch->comm), channel, message_on, message_off);
    return TRUE;
}
