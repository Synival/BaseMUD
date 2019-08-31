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

#include <stdlib.h>

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "fight.h"
#include "utils.h"
#include "chars.h"
#include "find.h"

#include "wiz_l3.h"

/* TODO: review most of these functions and test them thoroughly. */

void do_disconnect (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Disconnect whom?\n\r", ch);

    if (is_number (arg)) {
        int desc;

        desc = atoi (arg);
        for (d = descriptor_list; d != NULL; d = d->next) {
            if (d->descriptor == desc) {
                close_socket (d);
                send_to_char ("Ok.\n\r", ch);
                return;
            }
        }
    }

    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF_ACT (victim->desc == NULL,
        "$N doesn't have a descriptor.", ch, NULL, victim);

    for (d = descriptor_list; d != NULL; d = d->next) {
        if (d == victim->desc) {
            close_socket (d);
            send_to_char ("Ok.\n\r", ch);
            return;
        }
    }

    bug ("do_disconnect: desc not found.", 0);
    send_to_char ("Descriptor not found!\n\r", ch);
}

void do_pardon (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF (arg1[0] == '\0' || arg2[0] == '\0',
        "Syntax: pardon <character> <killer|thief>.\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);

    if (!str_cmp (arg2, "killer")) {
        if (IS_SET (victim->plr, PLR_KILLER)) {
            REMOVE_BIT (victim->plr, PLR_KILLER);
            send_to_char ("Killer flag removed.\n\r", ch);
            send_to_char ("You are no longer a KILLER.\n\r", victim);
        }
        return;
    }
    if (!str_cmp (arg2, "thief")) {
        if (IS_SET (victim->plr, PLR_THIEF)) {
            REMOVE_BIT (victim->plr, PLR_THIEF);
            send_to_char ("Thief flag removed.\n\r", ch);
            send_to_char ("You are no longer a THIEF.\n\r", victim);
        }
        return;
    }

    send_to_char ("Syntax: pardon <character> <killer|thief>.\n\r", ch);
}

void do_sla (CHAR_DATA * ch, char *argument)
    { send_to_char ("If you want to SLAY, spell it out.\n\r", ch); }

void do_slay (CHAR_DATA * ch, char *argument) {
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Slay whom?\n\r", ch);
    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (ch == victim,
        "Suicide is a mortal sin.\n\r", ch);
    BAIL_IF (!IS_NPC (victim) && victim->level >= char_get_trust (ch),
        "You failed.\n\r", ch);

    act3 ("{1You slay $M in cold blood!{x",
          "{1$n slays you in cold blood!{x",
          "{1$n slays $N in cold blood!{x", ch, NULL, victim, 0, POS_RESTING);
    raw_kill (victim);
}
