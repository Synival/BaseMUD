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
#include "globals.h"

#include "wiz_l7.h"

/* Thanks to Grodyn for pointing out bugs in this function. */
DEFINE_DO_FUN (do_force) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0' || argument[0] == '\0',
        "Force whom to do what?\n\r", ch);

    one_argument (argument, arg2);
    BAIL_IF (!str_cmp (arg2, "delete") || !str_prefix (arg2, "mob"),
        "That will NOT be done.\n\r", ch);

    sprintf (buf, "$n forces you to '%s'.", argument);

    /* Replaced original block with code by Edwin to keep from
     * corrupting pfiles in certain pet-infested situations.
     * JR -- 10/15/00 */
    if (!str_cmp (arg, "all")) {
        DESCRIPTOR_T *desc,*desc_next;
        BAIL_IF (char_get_trust(ch) < MAX_LEVEL - 3,
            "Not at your level!\n\r", ch);
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
        CHAR_T *vch;
        CHAR_T *vch_next;

        BAIL_IF (char_get_trust (ch) < MAX_LEVEL - 2,
            "Not at your level!\n\r", ch);
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
        CHAR_T *vch;
        CHAR_T *vch_next;

        BAIL_IF (char_get_trust (ch) < MAX_LEVEL - 2,
            "Not at your level!\n\r", ch);
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
        CHAR_T *victim;
        BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
            "They aren't here.\n\r", ch);
        BAIL_IF (victim == ch,
            "Aye aye, right away!\n\r", ch);

        BAIL_IF (!room_is_owner (victim->in_room, ch) &&
                ch->in_room != victim->in_room &&
                room_is_private (victim->in_room) &&
                !IS_TRUSTED (ch, IMPLEMENTOR),
            "That character is in a private room.\n\r", ch);

        BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
            "Do it yourself!\n\r", ch);
        BAIL_IF (!IS_NPC (victim) && char_get_trust (ch) < MAX_LEVEL - 3,
            "Not at your level!\n\r", ch);

        act (buf, ch, NULL, victim, TO_VICT);
        interpret (victim, argument);
    }
    send_to_char ("Ok.\n\r", ch);
}
