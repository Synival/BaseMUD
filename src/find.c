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
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 ***************************************************************************/

#include <stdlib.h>

#include "interp.h"
#include "utils.h"
#include "chars.h"
#include "db.h"
#include "globals.h"

#include "find.h"

int find_number_argument (const char *arg_in, char *arg_out) {
    int number;
    number = number_argument (arg_in, arg_out);
    find_stop_counting ();
    return number;
}

void find_continue_counting (void) {
    find_next_count = find_last_count;
}

void find_stop_counting (void) {
    find_next_count = 0;
    find_last_count = 0;
}

ROOM_INDEX_T *find_location (CHAR_T *ch, const char *arg) {
    CHAR_T *victim;
    OBJ_T *obj;

    if (is_number (arg))
        return get_room_index (atoi (arg));

    if ((victim = find_char_world (ch, arg)) != NULL)
        return victim->in_room;
    find_continue_counting();

    if ((obj = find_obj_world (ch, arg)) != NULL)
        return obj->in_room;

    return NULL;
}

CHAR_T *find_char_same_room (CHAR_T *ch, const char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *rch;
    int number;
    int count;

    count = find_next_count;
    number = find_number_argument (argument, arg);

    if (!str_cmp (arg, "self"))
        return ch;
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
        if (!char_can_see_in_room (ch, rch))
            continue;
        if (!is_name (arg, rch->name))
            continue;
        if (++count == number)
            break;
    }

    find_last_count = count;
    return rch;
}

CHAR_T *find_char_world (CHAR_T *ch, const char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *wch;
    int number;
    int count;

    if ((wch = find_char_same_room (ch, argument)) != NULL)
        return wch;
    find_continue_counting ();

    count = find_next_count;
    number = find_number_argument (argument, arg);

    for (wch = char_list; wch != NULL; wch = wch->next) {
        /* Skip characters that aren't in the game. */
        if (wch->in_room == NULL)
            continue;

        /* We already checked objects here; skip them! */
        if (wch->in_room == ch->in_room)
            continue;
        if (!is_name (arg, wch->name))
            continue;
        if (!char_can_see_anywhere (ch, wch))
            continue;
        if (++count == number)
            break;
    }

    find_last_count = count;
    return wch;
}

#define WORN_NO     -1
#define WORN_IGNORE  0
#define WORN_YES     1

OBJ_T *find_obj_room (CHAR_T *ch, ROOM_INDEX_T *room, const char *argument)
    { return find_obj_list (ch, room->contents, argument, WORN_IGNORE); }
OBJ_T *find_obj_same_room (CHAR_T *ch, const char *argument)
    { return find_obj_room (ch, ch->in_room, argument); }
OBJ_T *find_obj_container (CHAR_T *ch, OBJ_T *obj, const char *argument)
    { return find_obj_list (ch, obj->contains, argument, WORN_IGNORE); }

OBJ_T *find_obj_char (CHAR_T *ch, CHAR_T *victim, const char *argument)
    { return find_obj_list (ch, victim->carrying, argument, WORN_IGNORE); }
OBJ_T *find_obj_worn (CHAR_T *ch, CHAR_T *victim, const char *argument)
    { return find_obj_list (ch, victim->carrying, argument, WORN_YES); }
OBJ_T *find_obj_inventory (CHAR_T *ch, CHAR_T *victim, const char *argument)
    { return find_obj_list (ch, victim->carrying, argument, WORN_NO); }

OBJ_T *find_obj_own_char (CHAR_T *ch, const char *argument)
    { return find_obj_char (ch, ch, argument); }
OBJ_T *find_obj_own_inventory (CHAR_T *ch, const char *argument)
    { return find_obj_inventory (ch, ch, argument); }
OBJ_T *find_obj_own_worn (CHAR_T *ch, const char *argument)
    { return find_obj_worn (ch, ch, argument); }

OBJ_T *find_obj_list (CHAR_T *ch, OBJ_T *list, const char *argument, int worn) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    int number;
    int count;

    count = find_next_count;
    number = find_number_argument (argument, arg);

    for (obj = list; obj != NULL; obj = obj->next_content) {
        if (worn == WORN_NO && obj->wear_loc != WEAR_NONE)
            continue;
        if (worn == WORN_YES && obj->wear_loc == WEAR_NONE)
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            break;
    }

    find_last_count = count;
    return obj;
}

/* Find an obj in the room or in inventory. */
OBJ_T *find_obj_here (CHAR_T *ch, const char *argument) {
    OBJ_T *obj;

    if ((obj = find_obj_same_room (ch, argument)) != NULL)
        return obj;
    find_continue_counting ();

    if ((obj = find_obj_own_inventory (ch, argument)) != NULL)
        return obj;
    find_continue_counting ();

    if ((obj = find_obj_own_worn (ch, argument)) != NULL)
        return obj;

    return NULL;
}

/* Find an obj in the world. */
OBJ_T *find_obj_world (CHAR_T *ch, const char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    int number;
    int count;

    if ((obj = find_obj_here (ch, argument)) != NULL)
        return obj;
    find_continue_counting ();

    count = find_next_count;
    number = find_number_argument (argument, arg);

    for (obj = object_list; obj != NULL; obj = obj->next) {
        /* We already checked objects here; skip them! */
        if (obj->carried_by == ch || obj->in_room == ch->in_room)
            continue;

        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            break;
    }

    find_last_count = count;
    return obj;
}

/* get an object from a shopkeeper's list */
OBJ_T *find_obj_keeper (CHAR_T *ch, CHAR_T *keeper, const char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    int number;
    int count;

    count = find_next_count;
    number = find_number_argument (argument, arg);

    for (obj = keeper->carrying; obj != NULL; obj = obj->next_content) {
        if (obj->wear_loc != WEAR_NONE)
            continue;
        if (!char_can_see_obj (keeper, obj))
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            break;

        /* skip other objects of the same name */
        while (obj->next_content != NULL
               && obj->index_data == obj->next_content->index_data
               && !str_cmp (obj->short_descr, obj->next_content->short_descr))
            obj = obj->next_content;
    }

    find_last_count = count;
    return obj;
}

int find_door_same_room (CHAR_T *ch, const char *argument) {
    char arg[MAX_INPUT_LENGTH];
    EXIT_T *pexit;
    int door;
    int number;
    int count;

    count = find_next_count;
    number = find_number_argument (argument, arg);

    for (door = 0; door < DIR_MAX; door++) {
        if ((pexit = ch->in_room->exit[door]) == NULL)
            continue;
        if (!IS_SET (pexit->exit_flags, EX_ISDOOR))
            continue;
        if (pexit->keyword == NULL)
            continue;
        if (!is_name (arg, pexit->keyword))
            continue;
        if (++count == number)
            break;
    }
    if (door == DIR_MAX)
        door = -1;

    find_last_count = count;
    return door;
}
