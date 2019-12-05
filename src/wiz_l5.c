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
#include <stdlib.h>

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "save.h"
#include "fight.h"
#include "utils.h"
#include "lookup.h"
#include "skills.h"
#include "affects.h"
#include "do_sub.h"
#include "recycle.h"
#include "act_info.h"
#include "chars.h"
#include "objs.h"
#include "rooms.h"
#include "find.h"
#include "memory.h"
#include "globals.h"

#include "wiz_l5.h"

/* trust levels for load and clone */
bool do_obj_load_check (CHAR_T *ch, OBJ_T *obj) {
    if (IS_TRUSTED (ch, GOD))
        return TRUE;
    if (IS_TRUSTED (ch, IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
        return TRUE;
    if (IS_TRUSTED (ch, DEMI) && obj->level <= 10 && obj->cost <= 500)
        return TRUE;
    if (IS_TRUSTED (ch, ANGEL) && obj->level <= 5 && obj->cost <= 250)
        return TRUE;
    if (IS_TRUSTED (ch, AVATAR) && obj->level == 0 && obj->cost <= 100)
        return TRUE;
    return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void do_clone_recurse (CHAR_T *ch, OBJ_T *obj, OBJ_T *clone) {
    OBJ_T *c_obj, *t_obj;
    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content) {
        if (!do_obj_load_check (ch, c_obj))
            continue;

        t_obj = obj_create (c_obj->pIndexData, 0);
        obj_clone (c_obj, t_obj);
        obj_to_obj (t_obj, clone);
        do_clone_recurse (ch, c_obj, t_obj);
    }
}

/* RT nochannels command, for those spammers */
DEFINE_DO_FUN (do_nochannels) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Nochannel whom?", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
        "You failed.\n\r", ch);

    if (IS_SET (victim->comm, COMM_NOCHANNELS)) {
        REMOVE_BIT (victim->comm, COMM_NOCHANNELS);
        send_to_char ("The gods have restored your channel priviliges.\n\r",
                      victim);
        send_to_char ("NOCHANNELS removed.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N restores channels to %s", victim->name);
    }
    else {
        SET_BIT (victim->comm, COMM_NOCHANNELS);
        send_to_char ("The gods have revoked your channel priviliges.\n\r",
                      victim);
        send_to_char ("NOCHANNELS set.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N revokes %s's channels.", victim->name);
    }
}

DEFINE_DO_FUN (do_noemote) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Noemote whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
        "You failed.\n\r", ch);

    if (IS_SET (victim->comm, COMM_NOEMOTE)) {
        REMOVE_BIT (victim->comm, COMM_NOEMOTE);
        send_to_char ("You can emote again.\n\r", victim);
        send_to_char ("NOEMOTE removed.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N restores emotes to %s.", victim->name);
    }
    else {
        SET_BIT (victim->comm, COMM_NOEMOTE);
        send_to_char ("You can't emote!\n\r", victim);
        send_to_char ("NOEMOTE set.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N revokes %s's emotes.", victim->name);
    }
}

DEFINE_DO_FUN (do_noshout) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Noshout whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);
    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
        "You failed.\n\r", ch);

    if (IS_SET (victim->comm, COMM_NOSHOUT)) {
        REMOVE_BIT (victim->comm, COMM_NOSHOUT);
        send_to_char ("You can shout again.\n\r", victim);
        send_to_char ("NOSHOUT removed.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N restores shouts to %s.", victim->name);
    }
    else {
        SET_BIT (victim->comm, COMM_NOSHOUT);
        send_to_char ("You can't shout!\n\r", victim);
        send_to_char ("NOSHOUT set.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N revokes %s's shouts.", victim->name);
    }
}

DEFINE_DO_FUN (do_notell) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Notell whom?", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
        "You failed.\n\r", ch);

    if (IS_SET (victim->comm, COMM_NOTELL)) {
        REMOVE_BIT (victim->comm, COMM_NOTELL);
        send_to_char ("You can tell again.\n\r", victim);
        send_to_char ("NOTELL removed.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N restores tells to %s.", victim->name);
    }
    else {
        SET_BIT (victim->comm, COMM_NOTELL);
        send_to_char ("You can't tell!\n\r", victim);
        send_to_char ("NOTELL set.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N revokes %s's tells.", victim->name);
    }
}

DEFINE_DO_FUN (do_transfer) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_T *location;
    DESCRIPTOR_T *d;
    CHAR_T *victim;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    BAIL_IF (arg1[0] == '\0',
        "Transfer whom (and where)?\n\r", ch);

    if (!str_cmp (arg1, "all")) {
        for (d = descriptor_list; d != NULL; d = d->next) {
            if (d->connected == CON_PLAYING
                && d->character != ch
                && d->character->in_room != NULL
                && char_can_see_anywhere (ch, d->character))
            {
                char buf[MAX_STRING_LENGTH];
                sprintf (buf, "%s %s", d->character->name, arg2);
                do_function (ch, &do_transfer, buf);
            }
        }
        return;
    }

    /* Thanks to Grodyn for the optional location parameter. */
    if (arg2[0] == '\0')
        location = ch->in_room;
    else {
        BAIL_IF ((location = find_location (ch, arg2)) == NULL,
            "No such location.\n\r", ch);
        BAIL_IF (!room_is_owner (location, ch) && room_is_private (location) &&
                char_get_trust (ch) < MAX_LEVEL,
            "That room is private right now.\n\r", ch);
    }
    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim->in_room == NULL,
        "They are in limbo.\n\r", ch);

    if (victim->fighting != NULL)
        stop_fighting (victim, TRUE);
    act ("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_NOTCHAR);
    char_from_room (victim);
    char_to_room (victim, location);
    act ("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_NOTCHAR);
    if (ch != victim)
        act ("$n has transferred you.", ch, NULL, victim, TO_VICT);
    do_function (victim, &do_look, "auto");
    send_to_char ("Ok.\n\r", ch);
}

DEFINE_DO_FUN (do_peace) {
    CHAR_T *rch;

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
        if (rch->fighting != NULL)
            stop_fighting (rch, TRUE);
        if (IS_NPC (rch) && IS_SET (rch->mob, MOB_AGGRESSIVE))
            REMOVE_BIT (rch->mob, MOB_AGGRESSIVE);
    }
    send_to_char ("Ok.\n\r", ch);
}

DEFINE_DO_FUN (do_snoop) {
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_T *d;
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Snoop whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (victim->desc == NULL,
        "No descriptor to snoop.\n\r", ch);

    if (victim == ch) {
        send_to_char ("Cancelling all snoops.\n\r", ch);
        wiznet ("$N stops being such a snoop.",
                ch, NULL, WIZ_SNOOPS, WIZ_SECURE, char_get_trust (ch));
        for (d = descriptor_list; d != NULL; d = d->next)
            if (d->snoop_by == ch->desc)
                d->snoop_by = NULL;
        return;
    }
    BAIL_IF (victim->desc->snoop_by != NULL,
        "Busy already.\n\r", ch);

    BAIL_IF (!room_is_owner (victim->in_room, ch) &&
            ch->in_room != victim->in_room &&
            room_is_private (victim->in_room) &&
            !IS_TRUSTED (ch, IMPLEMENTOR),
        "That character is in a private room.\n\r", ch);

    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch) ||
            IS_SET (victim->comm, COMM_SNOOP_PROOF),
        "You failed.\n\r", ch);

    if (ch->desc != NULL)
        for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
            BAIL_IF (d->character == victim || d->original == victim,
                "No snoop loops.\n\r", ch);

    victim->desc->snoop_by = ch->desc;
    wiznetf (ch, NULL, WIZ_SNOOPS, WIZ_SECURE, char_get_trust (ch),
        "$N starts snooping on %s", PERS (victim));
    send_to_char ("Ok.\n\r", ch);
}

DEFINE_DO_FUN (do_string) {
    char type[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj;

    smash_tilde (argument);
    argument = one_argument (argument, type);
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    strcpy (arg3, argument);

    if (type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0'
        || arg3[0] == '\0')
    {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  string char <name> <field> <string>\n\r", ch);
        send_to_char ("    fields: name short long desc title spec\n\r", ch);
        send_to_char ("  string obj  <name> <field> <string>\n\r", ch);
        send_to_char ("    fields: name short long extended\n\r", ch);
        return;
    }

    if (!str_prefix (type, "character") || !str_prefix (type, "mobile")) {
        BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
            "They aren't here.\n\r", ch);

        /* clear zone for mobs */
        victim->zone = NULL;

        /* string something */
        if (!str_prefix (arg2, "name")) {
            BAIL_IF (!IS_NPC (victim),
                "Not on PC's.\n\r", ch);
            str_replace_dup (&(victim->name), arg3);
            return;
        }
        if (!str_prefix (arg2, "description")) {
            str_replace_dup (&(victim->description), arg3);
            return;
        }
        if (!str_prefix (arg2, "short")) {
            str_replace_dup (&(victim->short_descr), arg3);
            return;
        }
        if (!str_prefix (arg2, "long")) {
            strcat (arg3, "\n\r");
            str_replace_dup (&(victim->long_descr), arg3);
            return;
        }
        if (!str_prefix (arg2, "title")) {
            BAIL_IF (IS_NPC (victim),
                "Not on NPC's.\n\r", ch);
            char_set_title (victim, arg3);
            return;
        }
        if (!str_prefix (arg2, "spec")) {
            SPEC_FUN *spec_fun;
            BAIL_IF (!IS_NPC (victim),
                "Not on PC's.\n\r", ch);
            BAIL_IF ((spec_fun = spec_lookup_function (arg3)) == NULL,
                "No such spec fun.\n\r", ch);
            victim->spec_fun = spec_fun;
            return;
        }
    }

    if (!str_prefix (type, "object")) {
        /* string an obj */
        BAIL_IF ((obj = find_obj_world (ch, arg1)) == NULL,
            "Nothing like that in heaven or earth.\n\r", ch);

        if (!str_prefix (arg2, "name")) {
            str_replace_dup (&(obj->name), arg3);
            return;
        }
        if (!str_prefix (arg2, "short")) {
            str_replace_dup (&(obj->short_descr), arg3);
            return;
        }
        if (!str_prefix (arg2, "long")) {
            str_replace_dup (&(obj->description), arg3);
            return;
        }
        if (!str_prefix (arg2, "ed") || !str_prefix (arg2, "extended")) {
            EXTRA_DESCR_T *ed;

            argument = one_argument (argument, arg3);
            BAIL_IF (argument == NULL,
                "Syntax: oset <object> ed <keyword> <string>\n\r", ch);
            strcat (argument, "\n\r");

            ed = extra_descr_new ();
            ed->keyword = str_dup (arg3);
            ed->description = str_dup (argument);
            LIST_FRONT (ed, next, obj->extra_descr);
            return;
        }
    }

    /* echo bad use message */
    do_function (ch, &do_string, "");
}

/* command that is similar to load */
DEFINE_DO_FUN (do_clone) {
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_T *mob;
    OBJ_T *obj;

    rest = one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Clone what?\n\r", ch);

    if (!str_prefix (arg, "object")) {
        mob = NULL;
        BAIL_IF ((obj = find_obj_here (ch, rest)) == NULL,
            "You don't see that here.\n\r", ch);
    }
    else if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character")) {
        obj = NULL;
        BAIL_IF ((mob = find_char_same_room (ch, rest)) == NULL,
            "You don't see that here.\n\r", ch);
    }
    else { /* find both */
        mob = find_char_same_room (ch, argument);
        obj = find_obj_here (ch, argument);
        BAIL_IF (mob == NULL && obj == NULL,
            "You don't see that here.\n\r", ch);
    }

    /* clone an object */
    if (obj != NULL) {
        OBJ_T *clone;
        BAIL_IF (!do_obj_load_check (ch, obj),
            "Your powers are not great enough for such a task.\n\r", ch);

        clone = obj_create (obj->pIndexData, 0);
        obj_clone (obj, clone);
        if (obj->carried_by != NULL)
            obj_to_char (clone, ch);
        else
            obj_to_room (clone, ch->in_room);
        do_clone_recurse (ch, obj, clone);

        act2 ("You clone $p.", "$n has created $p.",
            ch, clone, NULL, 0, POS_RESTING);
        wiznet ("$N clones $p.", ch, clone, WIZ_LOAD, WIZ_SECURE,
                char_get_trust (ch));
        return;
    }

    if (mob != NULL) {
        CHAR_T *clone;
        OBJ_T *new_obj;

        BAIL_IF (!IS_NPC (mob),
            "You can only clone mobiles.\n\r", ch);

        BAIL_IF ((mob->level > 20 && !IS_TRUSTED (ch, GOD)) ||
                 (mob->level > 10 && !IS_TRUSTED (ch, IMMORTAL)) ||
                 (mob->level >  5 && !IS_TRUSTED (ch, DEMI)) ||
                 (mob->level >  0 && !IS_TRUSTED (ch, ANGEL)) ||
                 !IS_TRUSTED (ch, AVATAR),
            "Your powers are not great enough for such a task.\n\r", ch);

        clone = char_create_mobile (mob->pIndexData);
        char_clone_mobile (mob, clone);
        for (obj = mob->carrying; obj != NULL; obj = obj->next_content) {
            if (do_obj_load_check (ch, obj)) {
                new_obj = obj_create (obj->pIndexData, 0);
                obj_clone (obj, new_obj );
                do_clone_recurse (ch, obj, new_obj );
                obj_to_char (new_obj, clone);
                new_obj->wear_loc = obj->wear_loc;
            }
        }
        char_to_room (clone, ch->in_room);
        act2 ("You clone $N.", "$n has created $N.",
            ch, NULL, clone, 0, POS_RESTING);
        wiznetf (ch, NULL, WIZ_LOAD, WIZ_SECURE, char_get_trust (ch),
            "$N clones %s.", clone->short_descr);
        return;
    }
}

/* RT anti-newbie code */
DEFINE_DO_FUN (do_newlock) {
    extern bool newlock;
    newlock = !newlock;

    if (newlock) {
        wiznet ("$N locks out new characters.", ch, NULL, 0, 0, 0);
        send_to_char ("New characters have been locked out.\n\r", ch);
    }
    else {
        wiznet ("$N allows new characters back in.", ch, NULL, 0, 0, 0);
        send_to_char ("Newlock removed.\n\r", ch);
    }
}
