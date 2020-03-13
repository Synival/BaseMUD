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

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "chars.h"
#include "rooms.h"
#include "find.h"
#include "globals.h"
#include "memory.h"

#include "wiz_l6.h"

DEFINE_DO_FUN (do_at) {
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_T *location;
    ROOM_INDEX_T *original;
    OBJ_T *on;
    CHAR_T *wch;

    argument = one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0' || argument[0] == '\0',
        "At where what?\n\r", ch);
    BAIL_IF ((location = find_location (ch, arg)) == NULL,
        "No such location.\n\r", ch);
    BAIL_IF (!room_is_owner (location, ch) && room_is_private (location) &&
            char_get_trust (ch) < MAX_LEVEL,
        "That room is private right now.\n\r", ch);

    original = ch->in_room;
    on = ch->on;
    char_to_room (ch, location);
    interpret (ch, argument);

    /* See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case. */
    for (wch = char_first; wch; wch = wch->global_next) {
        if (wch == ch) {
            char_to_room (ch, original);
            ch->on = on;
            break;
        }
    }
}

DEFINE_DO_FUN (do_recho) {
    DESCRIPTOR_T *d;

    BAIL_IF (argument[0] == '\0',
        "Local echo what?\n\r", ch);

    for (d = descriptor_first; d; d = d->global_next) {
        if (d->connected != CON_PLAYING)
            continue;
        if (d->character->in_room != ch->in_room)
            continue;
        echo_to_char (d->character, ch, "local", argument);
    }
}

DEFINE_DO_FUN (do_return) {
    if (ch->desc == NULL)
        return;
    BAIL_IF (ch->desc->original == NULL,
        "You aren't switched.\n\r", ch);

    send_to_char ("You return to your original body. "
                  "Type replay to see any missed tells.\n\r", ch);

    if (ch->prompt != NULL)
        str_replace_dup (&(ch->prompt), NULL);

    wiznetf (ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
        char_get_trust (ch), "$N returns from %s.", ch->short_descr);
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
}

DEFINE_DO_FUN (do_switch) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Switch into whom?\n\r", ch);

    if (ch->desc == NULL)
        return;
    BAIL_IF (ch->desc->original != NULL,
        "You are already switched.\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim == ch,
        "Switch to yourself?\n\r", ch);
    BAIL_IF (!IS_NPC (victim),
        "You can only switch into mobiles.\n\r", ch);

    BAIL_IF (!room_is_owner (victim->in_room, ch) &&
            ch->in_room != victim->in_room &&
            room_is_private (victim->in_room) &&
            !IS_TRUSTED (ch, IMPLEMENTOR),
        "That character is in a private room.\n\r", ch);
    BAIL_IF (victim->desc != NULL,
        "Character in use.\n\r", ch);

    wiznetf (ch, NULL, WIZ_SWITCHES, WIZ_SECURE, char_get_trust (ch),
        "$N switches into %s", victim->short_descr);

    ch->desc->character = victim;
    ch->desc->original = ch;
    victim->desc = ch->desc;
    ch->desc = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup (ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    printf_to_char(victim, "Successfully switched into %s.\n\r", victim->short_descr);
}
