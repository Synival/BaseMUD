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

#include <string.h>

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "fight.h"
#include "utils.h"
#include "act_info.h"
#include "chars.h"
#include "rooms.h"
#include "find.h"
#include "memory.h"

#include "wiz_l8.h"

DEFINE_DO_FUN (do_goto) {
    ROOM_INDEX_T *location;
    CHAR_T *rch;
    int count = 0;

    BAIL_IF (argument[0] == '\0',
        "Goto where?\n\r", ch);
    BAIL_IF ((location = find_location (ch, argument)) == NULL,
        "No such location.\n\r", ch);

    count = 0;
    for (rch = location->people_first; rch; rch = rch->room_next)
        count++;

    BAIL_IF (!room_is_owner (location, ch) && room_is_private (location) &&
            (count > 1 || char_get_trust (ch) < MAX_LEVEL),
        "That room is private right now.\n\r", ch);

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    for (rch = ch->in_room->people_first; rch; rch = rch->room_next) {
        if (char_get_trust (rch) >= ch->invis_level) {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
            else
                act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
        }
    }

    char_to_room (ch, location);
    for (rch = ch->in_room->people_first; rch; rch = rch->room_next) {
        if (char_get_trust (rch) >= ch->invis_level) {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
            else
                act ("$n appears in a swirling mist.", ch, NULL, rch,
                     TO_VICT);
        }
    }

    do_function (ch, &do_look, "auto");
}

DEFINE_DO_FUN (do_bamfin) {
    if (IS_NPC (ch))
        return;

    str_smash_tilde (argument);
    if (argument[0] == '\0') {
        printf_to_char (ch, "Your poofin is %s\n\r", ch->pcdata->bamfin);
        return;
    }
    BAIL_IF (strstr (argument, ch->name) == NULL,
        "You must include your name.\n\r", ch);

    str_replace_dup (&(ch->pcdata->bamfin), argument);
    printf_to_char (ch, "Your poofin is now %s\n\r", ch->pcdata->bamfin);
}

DEFINE_DO_FUN (do_bamfout) {
    if (IS_NPC (ch))
        return;

    str_smash_tilde (argument);
    if (argument[0] == '\0') {
        printf_to_char (ch, "Your poofout is %s\n\r", ch->pcdata->bamfout);
        return;
    }
    BAIL_IF (strstr (argument, ch->name) == NULL,
        "You must include your name.\n\r", ch);

    str_replace_dup (&(ch->pcdata->bamfout), argument);
    printf_to_char (ch, "Your poofout is now %s\n\r", ch->pcdata->bamfout);
}
