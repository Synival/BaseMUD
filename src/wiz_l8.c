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

#include <string.h>

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "fight.h"
#include "utils.h"
#include "act_info.h"
#include "chars.h"
#include "rooms.h"

#include "wiz_l8.h"

/* TODO: review most of these functions and test them thoroughly. */
/* TODO: BAIL_IF() clauses. */
/* TODO: employ tables whenever possible */

void do_goto (CHAR_DATA * ch, char *argument) {
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if (argument[0] == '\0') {
        send_to_char ("Goto where?\n\r", ch);
        return;
    }
    if ((location = find_location (ch, argument)) == NULL) {
        send_to_char ("No such location.\n\r", ch);
        return;
    }

    count = 0;
    for (rch = location->people; rch != NULL; rch = rch->next_in_room)
        count++;

    if (!room_is_owner (location, ch) && room_is_private (location)
        && (count > 1 || char_get_trust (ch) < MAX_LEVEL))
    {
        send_to_char ("That room is private right now.\n\r", ch);
        return;
    }

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
        if (char_get_trust (rch) >= ch->invis_level) {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
            else
                act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
        }
    }

    char_from_room (ch);
    char_to_room (ch, location);

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
        if (char_get_trust (rch) >= ch->invis_level) {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
            else
                act ("$n appears in a swirling mist.", ch, NULL, rch,
                     TO_VICT);
        }
    }

    do_function (ch, &do_look, "auto");
    return;
}

void do_bamfin (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    if (IS_NPC (ch))
        return;

    smash_tilde (argument);
    if (argument[0] == '\0') {
        sprintf (buf, "Your poofin is %s\n\r", ch->pcdata->bamfin);
        send_to_char (buf, ch);
        return;
    }
    if (strstr (argument, ch->name) == NULL) {
        send_to_char ("You must include your name.\n\r", ch);
        return;
    }

    str_free (ch->pcdata->bamfin);
    ch->pcdata->bamfin = str_dup (argument);

    sprintf (buf, "Your poofin is now %s\n\r", ch->pcdata->bamfin);
    send_to_char (buf, ch);
}

void do_bamfout (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    if (IS_NPC (ch))
        return;

    smash_tilde (argument);
    if (argument[0] == '\0') {
        sprintf (buf, "Your poofout is %s\n\r", ch->pcdata->bamfout);
        send_to_char (buf, ch);
        return;
    }
    if (strstr (argument, ch->name) == NULL) {
        send_to_char ("You must include your name.\n\r", ch);
        return;
    }

    str_free (ch->pcdata->bamfout);
    ch->pcdata->bamfout = str_dup (argument);

    sprintf (buf, "Your poofout is now %s\n\r", ch->pcdata->bamfout);
    send_to_char (buf, ch);
}
