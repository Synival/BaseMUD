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

#include "wiz_l7.h"

#include "chars.h"
#include "chars.h"
#include "comm.h"
#include "find.h"
#include "globals.h"
#include "interp.h"
#include "objs.h"
#include "rooms.h"
#include "utils.h"

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
        for (desc = descriptor_first; desc != NULL; desc = desc_next) {
            desc_next = desc->global_next;
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
        for (vch = char_first; vch; vch = vch_next) {
            vch_next = vch->global_next;
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
        for (vch = char_first; vch; vch = vch_next) {
            vch_next = vch->global_next;
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
    printf_to_char(ch, "You forced %s to %s.\n\r", arg, argument);
}

/*
 * Confiscates an object from a player.  The command is useful for cases when
 * an immortal needs to get a noremove or nodrop item from a player to do
 * a restring or if they need to confiscate something for really any other
 * reason.
 *
 * Code by Keridan of Benevolent Iniquity, additions and conversions for
 * BaseMUD by Rhien (Blake Pell).
 */
DEFINE_DO_FUN (do_confiscate) {
    CHAR_T *victim;
    OBJ_T *obj;
    char arg1[MAX_INPUT_LENGTH];
    bool found = FALSE;

    argument = one_argument(argument, arg1);

    BAIL_IF (IS_NPC(ch),
        "Mobiles can't use confiscate command.\r\n", ch);
    BAIL_IF (IS_NULLSTR(argument) || IS_NULLSTR(arg1),
        "Syntax: confiscate <item> <player>\r\n", ch);
    BAIL_IF ((victim = find_char_world(ch, argument)) == NULL,
        "They aren't here.\r\n", ch);
    BAIL_IF (victim->level >= ch->level,
        "They are too high level for you to do that.\r\n", ch);

    /* Go through the victim's objects and find the first object that matches. */
    for (obj = victim->content_first; obj != NULL; obj = obj->content_next) {
        if (str_in_namelist (arg1, obj->name)) {
            found = TRUE;
            break;
        }
    }
    BAIL_IF (!found,
        "They do not have that item.\r\n", ch);

    /* Take the item from the character and give it to the immortal. */
    obj_take_from_char (obj);
    obj_give_to_char (obj, ch);

    /* For transparency both the immortal is notified what object was confiscated AND
     * the player is notified that it happened.  In the future perhaps a silent option
     * should be built in for cases where the immortal wants to confiscate something in a
     * quest and not disrupt the ambiance of whatever roleplay is occuring. */

    printf_to_char (ch, "You have confiscated %s from %s.\r\n",
        obj->short_descr, victim->name);
    printf_to_char (victim, "%s has confiscated %s from you.\r\n",
        PERS_AW (ch, victim), obj->short_descr);
}
