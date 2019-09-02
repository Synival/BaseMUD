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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_move.c,v 1.2 2000/12/01 10:48:33 ring0 Exp $ */

#include <string.h>

#include "interp.h"
#include "lookup.h"
#include "utils.h"
#include "comm.h"
#include "mob_prog.h"
#include "affects.h"
#include "skills.h"
#include "db.h"
#include "fight.h"
#include "groups.h"
#include "act_info.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"
#include "find.h"

#include "act_move.h"

int door_filter_find (CHAR_DATA *ch, char *argument) {
    char arg[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    int door;

    /* Lookup by direction. */
    if ((door = door_lookup (argument)) >= 0) {
        /* This method wants to continue from a previous find_() if looking
         * for a door by name, so we can assume there was a
         * find_continue_count() somewhere. This will "consume" it. */
        find_next_count = 0;

        RETURN_IF_ACT ((pexit = ch->in_room->exit[door]) == NULL,
            "You see no door $T here.", ch, NULL, NULL, -1);
        RETURN_IF_ACT (!IS_SET (pexit->exit_flags, EX_ISDOOR),
            "You can't do that.", ch, NULL, NULL, -1);
        return door;
    }

    /* Lookup by name. */
    RETURN_IF_ACT ((door == find_door_same_room (ch, argument)) == -1,
        "You see no $T here.", ch, NULL, arg, -1);
    return door;
}

bool door_filter_is_door (CHAR_DATA *ch, EXIT_DATA *pexit,
    OBJ_DATA *obj, flag_t *out_flags, bool *out_container, int *out_key)
{
    flag_t flags;
    bool container = FALSE;
    int key;

    /* Evaluate flags and flag types for objects. */
    if (obj) {
        switch (obj->item_type) {
            case ITEM_PORTAL:
                flags     = obj->value[1];
                container = FALSE;
                key       = obj->value[4];
                break;
            case ITEM_CONTAINER:
                flags     = obj->value[1];
                container = TRUE;
                key       = obj->value[2];
                break;
            default:
                send_to_char ("That's not a container.\n\r", ch);
                return TRUE;
        }
    }
    /* Evaluate flags and flag types of exits. */
    else if (pexit) {
        container = FALSE;
        flags     = pexit->exit_flags;
        key       = pexit->key;
    }
    /* Not sure what we're evaluating. */
    else {
        bug ("door_filter_can_open: No exit or object provided", 0);
        return TRUE;
    }

    /* Filter the flags we want. */
    FILTER (!IS_SET (flags, container ? CONT_CLOSEABLE : EX_ISDOOR),
        "You can't do that.\n\r", ch);

    /* Write output flags and return 'FALSE' to indicate 'not filtered'. */
    if (out_flags)     *out_flags     = flags;
    if (out_container) *out_container = container;
    if (out_key)       *out_key       = key;

    return FALSE;
}

bool door_filter_can_open (CHAR_DATA *ch, EXIT_DATA *pexit,
    OBJ_DATA *obj)
{
    flag_t flags;
    bool container;

    if (door_filter_is_door (ch, pexit, obj, &flags, &container, NULL))
        return TRUE;
    FILTER (!IS_SET (flags, container ? CONT_CLOSED : EX_CLOSED),
        "It's already open.\n\r", ch);
    FILTER (IS_SET (flags, container ? CONT_LOCKED : EX_LOCKED),
        "It's locked.\n\r", ch);
    return FALSE;
}

bool door_filter_can_close (CHAR_DATA *ch, EXIT_DATA *pexit,
    OBJ_DATA *obj)
{
    flag_t flags;
    bool container;

    if (door_filter_is_door (ch, pexit, obj, &flags, &container, NULL))
        return TRUE;
    FILTER (IS_SET (flags, container ? CONT_CLOSED : EX_CLOSED),
        "It's already closed.\n\r", ch);
    return FALSE;
}

bool door_filter_can_lock (CHAR_DATA *ch, EXIT_DATA *pexit,
    OBJ_DATA *obj)
{
    flag_t flags;
    bool container;
    int key;

    if (door_filter_is_door (ch, pexit, obj, &flags, &container, &key))
        return TRUE;
    FILTER (!IS_SET (flags, container ? CONT_CLOSED : EX_CLOSED),
        "It's not closed.\n\r", ch);
    FILTER (IS_SET (flags, container ? CONT_LOCKED : EX_LOCKED),
        "It's already locked.\n\r", ch);
    FILTER (key < 0,
        "It can't be locked.\n\r", ch);
    FILTER (!(IS_IMMORTAL (ch) || char_has_key (ch, key)),
        "You lack the key.\n\r", ch);
    return FALSE;
}

bool door_filter_can_unlock (CHAR_DATA *ch, EXIT_DATA *pexit,
    OBJ_DATA *obj)
{
    flag_t flags;
    bool container;
    int key;

    if (door_filter_is_door (ch, pexit, obj, &flags, &container, &key))
        return TRUE;
    FILTER (!IS_SET (flags, container ? CONT_CLOSED : EX_CLOSED),
        "It's not closed.\n\r", ch);
    FILTER (!IS_SET (flags, container ? CONT_LOCKED : EX_LOCKED),
        "It's already unlocked.\n\r", ch);
    FILTER (key < 0,
        "It can't be unlocked.\n\r", ch);
    FILTER (!(IS_IMMORTAL (ch) || char_has_key (ch, key)),
        "You lack the key.\n\r", ch);
    return FALSE;
}

bool door_filter_can_pick (CHAR_DATA *ch, EXIT_DATA *pexit,
    OBJ_DATA *obj)
{
    flag_t flags;
    bool container;
    int key;

    if (door_filter_is_door (ch, pexit, obj, &flags, &container, &key))
        return TRUE;
    FILTER (!IS_SET (flags, container ? CONT_CLOSED : EX_CLOSED),
        "It's not closed.\n\r", ch);
    FILTER (!IS_SET (flags, container ? CONT_LOCKED : EX_LOCKED),
        "It's already unlocked.\n\r", ch);
    FILTER (key < 0,
        "It can't be unlocked.\n\r", ch);

    /* look for guards, but not if it's ch's own object. */
    if (obj == NULL || obj->carried_by != ch) {
        CHAR_DATA *gch;
        for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
            if (IS_NPC (gch) && IS_AWAKE (gch) && ch->level + 5 < gch->level) {
                act ("$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR);
                return TRUE;
            }
        }
    }

    /* we're actually trying to pick something - make us wait. */
    WAIT_STATE (ch, skill_table[gsn_pick_lock].beats);

    /* pick-specific checks. */
    if (!IS_NPC (ch) && number_percent () > get_skill (ch, gsn_pick_lock)) {
        send_to_char ("You failed.\n\r", ch);
        check_improve (ch, gsn_pick_lock, FALSE, 2);
        return TRUE;
    }
    FILTER (IS_SET (flags, container ? CONT_PICKPROOF : EX_PICKPROOF),
        "You failed.\n\r", ch);
    return FALSE;
}

void do_open_object (CHAR_DATA *ch, OBJ_DATA *obj) {
    if (door_filter_can_open (ch, NULL, obj))
        return;
    switch (obj->item_type) {
        case ITEM_PORTAL:    REMOVE_BIT (obj->value[1], EX_CLOSED);   break;
        case ITEM_CONTAINER: REMOVE_BIT (obj->value[1], CONT_CLOSED); break;
    }
    act2 ("You open $p.", "$n opens $p.", ch, obj, NULL, 0, POS_RESTING);
}

void do_close_object (CHAR_DATA *ch, OBJ_DATA *obj) {
    if (door_filter_can_close (ch, NULL, obj))
        return;
    switch (obj->item_type) {
        case ITEM_PORTAL:    SET_BIT (obj->value[1], EX_CLOSED);   break;
        case ITEM_CONTAINER: SET_BIT (obj->value[1], CONT_CLOSED); break;
    }
    act2 ("You close $p.", "$n closes $p.", ch, obj, NULL, 0, POS_RESTING);
}

void do_unlock_object (CHAR_DATA *ch, OBJ_DATA *obj) {
    if (door_filter_can_unlock (ch, NULL, obj))
        return;
    switch (obj->item_type) {
        case ITEM_PORTAL:    REMOVE_BIT (obj->value[1], EX_LOCKED);   break;
        case ITEM_CONTAINER: REMOVE_BIT (obj->value[1], CONT_LOCKED); break;
    }
    act2 ("You unlock $p.", "$n unlocks $p.", ch, obj, NULL, 0, POS_RESTING);
}

void do_lock_object (CHAR_DATA *ch, OBJ_DATA *obj) {
    if (door_filter_can_lock (ch, NULL, obj))
        return;
    switch (obj->item_type) {
        case ITEM_PORTAL:    SET_BIT (obj->value[1], EX_LOCKED);   break;
        case ITEM_CONTAINER: SET_BIT (obj->value[1], CONT_LOCKED); break;
    }
    act2 ("You lock $p.", "$n locks $p.", ch, obj, NULL, 0, POS_RESTING);
}

void do_pick_object (CHAR_DATA *ch, OBJ_DATA *obj) {
    if (door_filter_can_pick (ch, NULL, obj))
        return;
    switch (obj->item_type) {
        case ITEM_PORTAL:    REMOVE_BIT (obj->value[1], EX_LOCKED);   break;
        case ITEM_CONTAINER: REMOVE_BIT (obj->value[1], CONT_LOCKED); break;
    }
    act2 ("You pick the lock on $p.", "$n picks the lock on $p.",
        ch, obj, NULL, 0, POS_RESTING);
    check_improve (ch, gsn_pick_lock, TRUE, 2);
}

void do_open_door (CHAR_DATA *ch, int door) {
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (door_filter_can_open (ch, pexit, NULL))
        return;

    REMOVE_BIT (pexit->exit_flags, EX_CLOSED);
    act2 ("You open the $d.", "$n opens the $d.",
        ch, NULL, pexit->keyword, 0, POS_RESTING);

    /* open the other side */
    if ((pexit_rev = room_get_opposite_exit (ch->in_room, door, NULL)) != NULL) {
        CHAR_DATA *rch;
        REMOVE_BIT (pexit_rev->exit_flags, EX_CLOSED);
        for (rch = pexit->to_room->people; rch != NULL; rch = rch->next_in_room)
            act ("The $d is opened from the other side.", rch, NULL,
                pexit_rev->keyword, TO_CHAR);
    }
}

void do_close_door (CHAR_DATA *ch, int door) {
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (door_filter_can_close (ch, pexit, NULL))
        return;

    SET_BIT (pexit->exit_flags, EX_CLOSED);
    act2 ("You close the $d.", "$n closes the $d.",
        ch, NULL, pexit->keyword, 0, POS_RESTING);

    /* close the other side */
    if ((pexit_rev = room_get_opposite_exit (ch->in_room, door, NULL)) != NULL) {
        CHAR_DATA *rch;
        SET_BIT (pexit_rev->exit_flags, EX_CLOSED);
        for (rch = pexit->to_room->people; rch != NULL; rch = rch->next_in_room)
            act ("The $d is closed from the other side.", rch, NULL,
                pexit_rev->keyword, TO_CHAR);
    }
}

void do_unlock_door (CHAR_DATA *ch, int door) {
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (door_filter_can_unlock (ch, pexit, NULL))
        return;

    REMOVE_BIT (pexit->exit_flags, EX_LOCKED);
    act2 ("*Click*", "$n unlocks the $d.",
        ch, NULL, pexit->keyword, 0, POS_RESTING);

    /* unlock the other side */
    if ((pexit_rev = room_get_opposite_exit (ch->in_room, door, NULL)) != NULL)
        REMOVE_BIT (pexit_rev->exit_flags, EX_LOCKED);
}

void do_lock_door (CHAR_DATA *ch, int door) {
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (door_filter_can_lock (ch, pexit, NULL))
        return;

    SET_BIT (pexit->exit_flags, EX_LOCKED);
    act2 ("*Click*", "$n locks the $d.",
        ch, NULL, pexit->keyword, 0, POS_RESTING);

    /* lock the other side */
    if ((pexit_rev = room_get_opposite_exit (ch->in_room, door, NULL)) != NULL)
        SET_BIT (pexit_rev->exit_flags, EX_LOCKED);
}

void do_pick_door (CHAR_DATA *ch, int door) {
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if (door_filter_can_pick (ch, pexit, NULL))
        return;

    REMOVE_BIT (pexit->exit_flags, EX_LOCKED);
    act2 ("*Click*", "$n picks the lock on $d.",
        ch, NULL, pexit->keyword, 0, POS_RESTING);
    check_improve (ch, gsn_pick_lock, TRUE, 2);

    /* unlock the other side */
    if ((pexit_rev = room_get_opposite_exit (ch->in_room, door, NULL)) != NULL)
        REMOVE_BIT (pexit_rev->exit_flags, EX_LOCKED);
}

void do_door (CHAR_DATA *ch, char *argument, char *arg_msg,
    void (*func_obj)  (CHAR_DATA *, OBJ_DATA *),
    void (*func_door) (CHAR_DATA *, int))
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char (arg_msg, ch);
        return;
    }

    if ((obj = find_obj_here (ch, arg)) != NULL)
        func_obj (ch, obj);
    else {
        find_continue_counting ();
        if ((door = door_filter_find (ch, arg)) >= 0)
            func_door (ch, door);
    }
}

void do_north (CHAR_DATA * ch, char *argument)
    { char_move (ch, DIR_NORTH, FALSE); }
void do_east (CHAR_DATA * ch, char *argument)
    { char_move (ch, DIR_EAST, FALSE); }
void do_south (CHAR_DATA * ch, char *argument)
    { char_move (ch, DIR_SOUTH, FALSE); }
void do_west (CHAR_DATA * ch, char *argument)
    { char_move (ch, DIR_WEST, FALSE); }
void do_up (CHAR_DATA * ch, char *argument)
    { char_move (ch, DIR_UP, FALSE); }
void do_down (CHAR_DATA * ch, char *argument)
    { char_move (ch, DIR_DOWN, FALSE); }

void do_open (CHAR_DATA * ch, char *argument)
    { do_door (ch, argument, "Open what?\n\r",   do_open_object,   do_open_door); }
void do_close (CHAR_DATA * ch, char *argument)
    { do_door (ch, argument, "Close what?\n\r",  do_close_object,  do_close_door); }
void do_unlock (CHAR_DATA * ch, char *argument)
    { do_door (ch, argument, "Unlock what?\n\r", do_unlock_object, do_unlock_door); }
void do_lock (CHAR_DATA * ch, char *argument)
    { do_door (ch, argument, "Lock what?\n\r",   do_lock_object,   do_lock_door); }
void do_pick (CHAR_DATA * ch, char *argument)
    { do_door (ch, argument, "Pick what?\n\r",   do_pick_object,   do_pick_door); }

bool do_filter_change_position (CHAR_DATA *ch, int pos, char *same_msg) {
    FILTER (ch->position == pos,
        same_msg, ch);
    FILTER (ch->position == POS_SLEEPING && IS_AFFECTED (ch, AFF_SLEEP),
        "You can't wake up!\n\r", ch);
    FILTER (ch->position == POS_FIGHTING,
        "Maybe you should finish this fight first?\n\r", ch);
    FILTER (ch->daze > 0,
        "You're too dazed to re-orient yourself right now!\n\r", ch);
    return FALSE;
}

void do_stand (CHAR_DATA * ch, char *argument) {
    OBJ_DATA *obj = NULL;
    int new_pos;

    BAIL_IF (ch->position == POS_SLEEPING && IS_AFFECTED (ch, AFF_SLEEP),
        "You can't wake up!\n\r", ch);

    /* For sit/rest/sleep, players need to stand up first to change their
     * position to another object. For standing, however, we don't, so the
     * logic is a bit different. -- Synival */
    if (argument[0] == '\0') {
        if (ch->on) {
            BAIL_IF (ch->position == POS_FIGHTING,
                "Maybe you should finish fighting first?\n\r", ch);
        }
        else {
            BAIL_IF (ch->position == POS_FIGHTING,
                "You are already fighting!\n\r", ch);
            BAIL_IF (ch->position == POS_STANDING,
                "You are already standing.\n\r", ch);
        }
        BAIL_IF (ch->daze > 0,
            "You're too dazed to re-orient yourself right now!\n\r", ch);
    }
    else {
        BAIL_IF (ch->position == POS_FIGHTING,
            "Maybe you should finish fighting first?\n\r", ch);
        BAIL_IF (ch->daze > 0,
            "You're too dazed to re-orient yourself right now!\n\r", ch);

        obj = find_obj_same_room (ch, argument);
        BAIL_IF (obj == NULL,
            "You don't see that here.\n\r", ch);
        BAIL_IF (!obj_is_furniture(obj, STAND_BITS),
            "You can't seem to find a place to stand.\n\r", ch);
        BAIL_IF_ACT (ch->on != obj && obj_count_users (obj) >= obj->value[0],
            "There's no room to stand on $p.", ch, obj, NULL);
    }

    /* If we're fighting someone, move to fighting position instead. */
    new_pos = ch->fighting ? POS_FIGHTING : POS_STANDING;

    BAIL_IF (!position_change_message(ch, ch->position, new_pos, obj),
        "You can't stand up from your current position.\n\r", ch);
    ch->position = new_pos;
    ch->on = obj;
}

void do_rest (CHAR_DATA * ch, char *argument) {
    OBJ_DATA *obj = NULL;

    if (do_filter_change_position (ch, POS_RESTING,
            "You are already resting.\n\r"))
        return;

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0') {
        obj = find_obj_same_room (ch, argument);
        BAIL_IF (obj == NULL,
            "You don't see that here.\n\r", ch);
    }
    else
        obj = ch->on;

    if (obj != NULL) {
        BAIL_IF (!obj_is_furniture(obj, REST_BITS),
            "You can't rest on that.\n\r", ch);
        BAIL_IF_ACT (ch->on != obj && obj_count_users (obj) >= obj->value[0],
            "There's no more room on $p.", ch, obj, NULL);
    }

    BAIL_IF (!position_change_message(ch, ch->position, POS_RESTING, obj),
        "You can't rest from your current position.\n\r", ch);
    ch->position = POS_RESTING;
    ch->on = obj;
}

void do_sit (CHAR_DATA * ch, char *argument) {
    OBJ_DATA *obj = NULL;

    if (do_filter_change_position (ch, POS_SITTING,
            "You are already sitting down.\n\r"))
        return;

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0') {
        obj = find_obj_same_room (ch, argument);
        BAIL_IF (obj == NULL,
            "You don't see that here.\n\r", ch);
    }
    else
        obj = ch->on;

    if (obj != NULL) {
        BAIL_IF (!obj_is_furniture(obj, SIT_BITS),
            "You can't sit on that.\n\r", ch);
        BAIL_IF_ACT (ch->on != obj && obj_count_users (obj) >= obj->value[0],
            "There's no more room on $p.", ch, obj, NULL);
    }

    BAIL_IF (!position_change_message(ch, ch->position, POS_SITTING, obj),
        "You can't sit from your current position.\n\r", ch);
    ch->position = POS_SITTING;
    ch->on = obj;
}

void do_sleep (CHAR_DATA * ch, char *argument) {
    OBJ_DATA *obj = NULL;

    if (do_filter_change_position (ch, POS_SLEEPING,
            "You are already sleeping.\n\r"))
        return;

    /* okay, now that we know we can sleep, find an object to sleep on */
    if (argument[0] != '\0') {
        obj = find_obj_same_room (ch, argument);
        BAIL_IF (obj == NULL,
            "You don't see that here.\n\r", ch);
    }
    else
        obj = ch->on;

    if (obj != NULL) {
        BAIL_IF (!obj_is_furniture(obj, SLEEP_BITS),
            "You can't sleep on that!\n\r", ch);
        BAIL_IF_ACT (ch->on != obj && obj_count_users (obj) >= obj->value[0],
            "There's no more room on $p.", ch, obj, NULL);
    }

    BAIL_IF (!position_change_message(ch, ch->position, POS_SLEEPING, obj),
        "You can't sleep from your current position.\n\r", ch);
    ch->position = POS_SLEEPING;
    ch->on = obj;
}

void do_wake (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        do_function (ch, &do_stand, "");
        return;
    }

    BAIL_IF (!IS_AWAKE (ch),
        "You are asleep yourself!\n\r", ch);
    BAIL_IF ((victim = find_char_same_room (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF_ACT (IS_AWAKE (victim),
        "$N is already awake.", ch, NULL, victim);
    BAIL_IF_ACT (IS_AFFECTED (victim, AFF_SLEEP),
        "You can't wake $M!", ch, NULL, victim);

    act3 ("You wake $N.", "$n wakes you.", "$n wakes $N.",
        ch, NULL, victim, 0, POS_SLEEPING);
    do_function (victim, &do_stand, "");
}

void do_sneak (CHAR_DATA * ch, char *argument) {
    AFFECT_DATA af;

    send_to_char ("You attempt to move silently.\n\r", ch);
    affect_strip (ch, gsn_sneak);

    if (IS_AFFECTED (ch, AFF_SNEAK))
        return;
    if (number_percent () < get_skill (ch, gsn_sneak)) {
        check_improve (ch, gsn_sneak, TRUE, 3);
        affect_init (&af, TO_AFFECTS, gsn_sneak, ch->level, ch->level, APPLY_NONE, 0, AFF_SNEAK);
        affect_to_char (ch, &af);
    }
    else
        check_improve (ch, gsn_sneak, FALSE, 3);
}

void do_hide (CHAR_DATA * ch, char *argument) {
    send_to_char ("You attempt to hide.\n\r", ch);

    if (IS_AFFECTED (ch, AFF_HIDE))
        REMOVE_BIT (ch->affected_by, AFF_HIDE);
    if (number_percent () < get_skill (ch, gsn_hide)) {
        SET_BIT (ch->affected_by, AFF_HIDE);
        check_improve (ch, gsn_hide, TRUE, 3);
    }
    else
        check_improve (ch, gsn_hide, FALSE, 3);
}

/* Contributed by Alander. */
void do_visible (CHAR_DATA * ch, char *argument) {
    affect_strip (ch, gsn_invis);
    affect_strip (ch, gsn_mass_invis);
    affect_strip (ch, gsn_sneak);
    REMOVE_BIT (ch->affected_by, AFF_HIDE);
    REMOVE_BIT (ch->affected_by, AFF_INVISIBLE);
    REMOVE_BIT (ch->affected_by, AFF_SNEAK);
    send_to_char ("Ok.\n\r", ch);
}

void do_recall (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    BAIL_IF (IS_NPC (ch) && !IS_PET (ch),
        "Only players can recall.\n\r", ch);

    act ("$n prays for transportation!", ch, NULL, NULL, TO_NOTCHAR);
    BAIL_IF ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL,
        "You are completely lost.\n\r", ch);
#ifndef VANILLA
    BAIL_IF (ch->in_room == location,
        "Mota ignores your frivolous request.\n\r", ch);
#endif
    BAIL_IF (IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL) ||
             IS_AFFECTED (ch, AFF_CURSE),
        "Mota has forsaken you.\n\r", ch);

    if ((victim = ch->fighting) != NULL) {
        int lose, skill;
        skill = get_skill (ch, gsn_recall);

        if (number_percent () < 80 * skill / 100) {
            check_improve (ch, gsn_recall, FALSE, 6);
            WAIT_STATE (ch, 4);
            sprintf (buf, "You failed!.\n\r");
            send_to_char (buf, ch);
            return;
        }

        lose = (ch->desc != NULL) ? 25 : 50;
        gain_exp (ch, 0 - lose);
        check_improve (ch, gsn_recall, TRUE, 4);
        sprintf (buf, "You recall from combat!  You lose %d exps.\n\r", lose);
        send_to_char (buf, ch);
        stop_fighting (ch, TRUE);
    }

    ch->move /= 2;
    act ("$n disappears.", ch, NULL, NULL, TO_NOTCHAR);
    char_from_room (ch);
    char_to_room (ch, location);
    act ("$n appears in the room.", ch, NULL, NULL, TO_NOTCHAR);
    do_function (ch, &do_look, "auto");

    if (ch->pet != NULL)
        do_function (ch->pet, &do_recall, "");
}

/* RT Enter portals */
void do_enter (CHAR_DATA * ch, char *argument) {
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *old_room;
    OBJ_DATA *portal;
    CHAR_DATA *fch, *fch_next;
    char *msg;

    /* Basic character / command checks. */
    BAIL_IF (ch->fighting != NULL,
        "Maybe finish the fight first?!\n\r", ch);
    BAIL_IF (argument[0] == '\0',
        "Nope, can't do it.\n\r", ch);

    /* Make sure we have a valid portal. */
    portal = find_obj_same_room (ch, argument);
    BAIL_IF (portal == NULL,
        "You don't see that here.\n\r", ch);
    BAIL_IF (portal->item_type != ITEM_PORTAL ||
            (IS_SET (portal->value[1], EX_CLOSED) && !IS_TRUSTED (ch, ANGEL)),
        "You can't seem to find a way in.\n\r", ch);
    BAIL_IF (!IS_TRUSTED (ch, ANGEL) &&
             !IS_SET (portal->value[2], GATE_NOCURSE) &&
             (IS_AFFECTED (ch, AFF_CURSE) ||
              IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)),
        "Something prevents you from leaving...\n\r", ch);

    /* Determine the target room. */
    if (IS_SET (portal->value[2], GATE_RANDOM) || portal->value[3] == -1) {
        location = get_random_room (ch);
        portal->value[3] = location->vnum; /* for record keeping :) */
    }
    else if (IS_SET (portal->value[2], GATE_BUGGY) && (number_percent () < 5))
        location = get_random_room (ch);
    else
        location = get_room_index (portal->value[3]);

    /* Check if the target room if valid. */
    BAIL_IF_ACT (location == NULL || location == ch->in_room ||
            !char_can_see_room (ch, location) ||
            (room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR)),
        "$p doesn't seem to go anywhere.", ch, portal, NULL);
    BAIL_IF (IS_NPC (ch) && IS_SET (ch->mob, MOB_AGGRESSIVE) &&
             IS_SET (location->room_flags, ROOM_LAW),
        "Something prevents you from leaving...\n\r", ch);

    /* We're leaving! Outgoing message. */
    msg = IS_SET (portal->value[2], GATE_NORMAL_EXIT)
        ? "You enter $p."
        : "You walk through $p and find yourself somewhere else...";
    act2 (msg, "$n steps into $p.", ch, portal, NULL, 0, POS_RESTING);

    /* Leave, take the portal along if specified. */
    old_room = ch->in_room;
    char_from_room (ch);
    char_to_room (ch, location);
    if (IS_SET (portal->value[2], GATE_GOWITH)) {
        obj_from_room (portal);
        obj_to_room (portal, location);
    }

    /* Arrival messages. */
    msg = IS_SET (portal->value[2], GATE_NORMAL_EXIT)
        ? "$n has arrived."
        : "$n has arrived through $p.";
    act (msg, ch, portal, NULL, TO_NOTCHAR);
    do_function (ch, &do_look, "auto");

    /* Charges. Zero charges = infinite uses. */
    if (portal->value[0] > 0) {
        portal->value[0]--;
        if (portal->value[0] == 0)
            portal->value[0] = -1;
    }

    /* Perform follows. */
    if (old_room != location) {
        for (fch = old_room->people; fch != NULL; fch = fch_next) {
            fch_next = fch->next_in_room;

            if (portal == NULL || portal->value[0] == -1)
                /* no following through dead portals */
                continue;

            if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
                && fch->position < POS_STANDING)
                do_function (fch, &do_stand, "");

            if (fch->master == ch && fch->position == POS_STANDING) {
                if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
                    && (IS_NPC (fch) && IS_SET (fch->mob, MOB_AGGRESSIVE)))
                {
                    act ("You can't bring $N into the city.",
                         ch, NULL, fch, TO_CHAR);
                    act ("You aren't allowed in the city.",
                         fch, NULL, NULL, TO_CHAR);
                    continue;
                }

                act ("You follow $N.", fch, NULL, ch, TO_CHAR);
                do_function (fch, &do_enter, argument);
            }
        }
    }

    /* If the portal is defunct, destroy it now. */
    if (portal != NULL && portal->value[0] == -1) {
        act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
        if (ch->in_room == old_room)
            act ("$p fades out of existence.", ch, portal, NULL, TO_NOTCHAR);
        else if (old_room->people != NULL)
            act ("$p fades out of existence.", old_room->people, portal, NULL, TO_ALL);
        obj_extract (portal);
    }

    /* If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way... */
    if (IS_NPC (ch) && HAS_TRIGGER (ch, TRIG_ENTRY))
        mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_ENTRY);
    if (!IS_NPC (ch))
        mp_greet_trigger (ch);
}
