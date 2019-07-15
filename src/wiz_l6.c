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

#include "wiz_l6.h"

/* TODO: review most of these functions and test them thoroughly. */
/* TODO: BAIL_IF() clauses. */
/* TODO: employ tables whenever possible */

void do_at (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;

    argument = one_argument (argument, arg);
    if (arg[0] == '\0' || argument[0] == '\0') {
        send_to_char ("At where what?\n\r", ch);
        return;
    }
    if ((location = find_location (ch, arg)) == NULL) {
        send_to_char ("No such location.\n\r", ch);
        return;
    }
    if (!room_is_owner (location, ch) && room_is_private (location)
        && char_get_trust (ch) < MAX_LEVEL)
    {
        send_to_char ("That room is private right now.\n\r", ch);
        return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room (ch);
    char_to_room (ch, location);
    interpret (ch, argument);

    /* See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case. */
    for (wch = char_list; wch != NULL; wch = wch->next) {
        if (wch == ch) {
            char_from_room (ch);
            char_to_room (ch, original);
            ch->on = on;
            break;
        }
    }
}

void do_recho (CHAR_DATA * ch, char *argument) {
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
        send_to_char ("Local echo what?\n\r", ch);
        return;
    }
    for (d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING
            && d->character->in_room == ch->in_room)
        {
            if (char_get_trust (d->character) >= char_get_trust (ch))
                send_to_char ("local> ", d->character);
            send_to_char (argument, d->character);
            send_to_char ("\n\r", d->character);
        }
    }
}

void do_return (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];

    if (ch->desc == NULL)
        return;
    if (ch->desc->original == NULL) {
        send_to_char ("You aren't switched.\n\r", ch);
        return;
    }

    send_to_char
        ("You return to your original body. Type replay to see any missed tells.\n\r",
         ch);
    if (ch->prompt != NULL) {
        str_free (ch->prompt);
        ch->prompt = NULL;
    }

    sprintf (buf, "$N returns from %s.", ch->short_descr);
    wiznet (buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
            char_get_trust (ch));
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
}

void do_switch (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Switch into whom?\n\r", ch);
        return;
    }
    if (ch->desc == NULL)
        return;

    if (ch->desc->original != NULL) {
        send_to_char ("You are already switched.\n\r", ch);
        return;
    }
    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }
    if (victim == ch) {
        send_to_char ("Ok.\n\r", ch);
        return;
    }
    if (!IS_NPC (victim)) {
        send_to_char ("You can only switch into mobiles.\n\r", ch);
        return;
    }

    if (!room_is_owner (victim->in_room, ch) && ch->in_room != victim->in_room
        && room_is_private (victim->in_room) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
        send_to_char ("That character is in a private room.\n\r", ch);
        return;
    }
    if (victim->desc != NULL) {
        send_to_char ("Character in use.\n\r", ch);
        return;
    }

    sprintf (buf, "$N switches into %s", victim->short_descr);
    wiznet (buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, char_get_trust (ch));

    ch->desc->character = victim;
    ch->desc->original = ch;
    victim->desc = ch->desc;
    ch->desc = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup (ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char ("Ok.\n\r", victim);
}
