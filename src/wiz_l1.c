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
#include "save.h"
#include "fight.h"
#include "ban.h"
#include "utils.h"
#include "act_player.h"
#include "wiz_l4.h"
#include "chars.h"
#include "find.h"

#include "wiz_l1.h"

/* TODO: review most of these functions and test them thoroughly. */

void do_deny (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Deny whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);
    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
        "You failed.\n\r", ch);

    SET_BIT (victim->plr, PLR_DENY);
    send_to_char ("You are denied access!\n\r", victim);
    sprintf (buf, "$N denies access to %s", victim->name);
    wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    send_to_char ("OK.\n\r", ch);

    save_char_obj (victim);
    stop_fighting (victim, TRUE);
    do_function (victim, &do_quit, "");
}

void do_permban (CHAR_DATA * ch, char *argument)
    { ban_site (ch, argument, TRUE); }

void do_protect (CHAR_DATA * ch, char *argument) {
    CHAR_DATA *victim;

    BAIL_IF (argument[0] == '\0',
        "Protect whom from snooping?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, argument)) == NULL,
        "You can't find them.\n\r", ch);

    if (IS_SET (victim->comm, COMM_SNOOP_PROOF)) {
        act_new ("$N is no longer snoop-proof.", ch, NULL, victim, TO_CHAR,
                 POS_DEAD);
        send_to_char ("Your snoop-proofing was just removed.\n\r", victim);
        REMOVE_BIT (victim->comm, COMM_SNOOP_PROOF);
    }
    else {
        act_new ("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
                 POS_DEAD);
        send_to_char ("You are now immune to snooping.\n\r", victim);
        SET_BIT (victim->comm, COMM_SNOOP_PROOF);
    }
}

void do_reboo (CHAR_DATA * ch, char *argument) {
    send_to_char ("If you want to REBOOT, spell it out.\n\r", ch);
    return;
}

void do_reboot (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO) {
        sprintf (buf, "Reboot by %s.", ch->name);
        do_function (ch, &do_echo, buf);
    }

    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next) {
        d_next = d->next;
        vch = d->original ? d->original : d->character;
        if (vch != NULL)
            save_char_obj (vch);
        close_socket (d);
    }
}

void do_shutdow (CHAR_DATA * ch, char *argument) {
    send_to_char ("If you want to SHUTDOWN, spell it out.\n\r", ch);
    return;
}

void do_shutdown (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
        sprintf (buf, "Shutdown by %s.", ch->name);
    append_file (ch, SHUTDOWN_FILE, buf);
    strcat (buf, "\n\r");
    if (ch->invis_level < LEVEL_HERO)
        do_function (ch, &do_echo, buf);
    merc_down = TRUE;
    for (d = descriptor_list; d != NULL; d = d_next) {
        d_next = d->next;
        vch = d->original ? d->original : d->character;
        if (vch != NULL)
            save_char_obj (vch);
        close_socket (d);
    }
}

void do_log (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Log whom?\n\r", ch);

    if (!str_cmp (arg, "all")) {
        if (fLogAll) {
            fLogAll = FALSE;
            send_to_char ("Log ALL off.\n\r", ch);
        }
        else {
            fLogAll = TRUE;
            send_to_char ("Log ALL on.\n\r", ch);
        }
        return;
    }

    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);

    /* No level check, gods can log anyone. */
    if (IS_SET (victim->plr, PLR_LOG)) {
        REMOVE_BIT (victim->plr, PLR_LOG);
        send_to_char ("LOG removed.\n\r", ch);
    }
    else {
        SET_BIT (victim->plr, PLR_LOG);
        send_to_char ("LOG set.\n\r", ch);
    }
}
