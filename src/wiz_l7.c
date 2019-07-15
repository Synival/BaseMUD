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

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "utils.h"
#include "chars.h"
#include "rooms.h"
#include "find.h"

#include "wiz_l7.h"

/* TODO: review most of these functions and test them thoroughly. */
/* TODO: BAIL_IF() clauses. */
/* TODO: employ tables whenever possible */

/* Thanks to Grodyn for pointing out bugs in this function. */
void do_force (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char ("Force whom to do what?\n\r", ch);
        return;
    }

    one_argument (argument, arg2);
    if (!str_cmp (arg2, "delete") || !str_prefix (arg2, "mob")) {
        send_to_char ("That will NOT be done.\n\r", ch);
        return;
    }

    sprintf (buf, "$n forces you to '%s'.", argument);

    /* Replaced original block with code by Edwin to keep from
     * corrupting pfiles in certain pet-infested situations.
     * JR -- 10/15/00
     */
    if (!str_cmp( arg, "all")) {
        DESCRIPTOR_DATA *desc,*desc_next;
        if (char_get_trust(ch) < MAX_LEVEL - 3) {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
        for (desc = descriptor_list; desc != NULL; desc = desc_next) {
            desc_next = desc->next;
            if (desc->connected == CON_PLAYING &&
                char_get_trust (desc->character) < char_get_trust (ch))
            {
                act (buf, ch, NULL, desc->character, TO_VICT);
                interpret (desc->character, argument);
            }
        }
    }
    else if (!str_cmp (arg, "players")) {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (char_get_trust (ch) < MAX_LEVEL - 2) {
            send_to_char ("Not at your level!\n\r", ch);
            return;
        }
        for (vch = char_list; vch != NULL; vch = vch_next) {
            vch_next = vch->next;
            if (!IS_NPC (vch) && char_get_trust (vch) < char_get_trust (ch)
                && vch->level < LEVEL_HERO)
            {
                act (buf, ch, NULL, vch, TO_VICT);
                interpret (vch, argument);
            }
        }
    }
    else if (!str_cmp (arg, "gods")) {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (char_get_trust (ch) < MAX_LEVEL - 2) {
            send_to_char ("Not at your level!\n\r", ch);
            return;
        }
        for (vch = char_list; vch != NULL; vch = vch_next) {
            vch_next = vch->next;

            if (!IS_NPC (vch) && char_get_trust (vch) < char_get_trust (ch)
                && vch->level >= LEVEL_HERO)
            {
                act (buf, ch, NULL, vch, TO_VICT);
                interpret (vch, argument);
            }
        }
    }
    else {
        CHAR_DATA *victim;
        if ((victim = find_char_world (ch, arg)) == NULL) {
            send_to_char ("They aren't here.\n\r", ch);
            return;
        }
        if (victim == ch) {
            send_to_char ("Aye aye, right away!\n\r", ch);
            return;
        }

        if (!room_is_owner (victim->in_room, ch)
            && ch->in_room != victim->in_room
            && room_is_private (victim->in_room)
            && !IS_TRUSTED (ch, IMPLEMENTOR))
        {
            send_to_char ("That character is in a private room.\n\r", ch);
            return;
        }
        if (char_get_trust (victim) >= char_get_trust (ch)) {
            send_to_char ("Do it yourself!\n\r", ch);
            return;
        }
        if (!IS_NPC (victim) && char_get_trust (ch) < MAX_LEVEL - 3) {
            send_to_char ("Not at your level!\n\r", ch);
            return;
        }

        act (buf, ch, NULL, victim, TO_VICT);
        interpret (victim, argument);
    }
    send_to_char ("Ok.\n\r", ch);
}
