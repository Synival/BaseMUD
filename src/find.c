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

#include "interp.h"
#include "utils.h"
#include "chars.h"
#include "db.h"
#include "act_comm.h"

#include "find.h"

/* TODO: find_obj_here() doesn't properly take 1., 2., etc. into account.
 *       it SHOULD build a large list of potential items and search through
 *       that. */
/* TODO: funnel ALL find_XXX() functions to 'find_XXX_array', which then
 *       handles all the 1., 2., etc. */
/* TODO: write 'find_XXX_array_many' that can handle 'all' and the
 *       'all.' prefix */

/* Find a char in the room. */
CHAR_DATA *find_char_room (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
    if (!str_cmp (arg, "self"))
        return ch;
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
        if (!char_can_see_in_room (ch, rch))
            continue;
        if (!is_name (arg, rch->name))
            continue;
        if (++count == number)
            return rch;
    }
    return NULL;
}

/* Find a char in the world. */
CHAR_DATA *find_char_world (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ((wch = find_char_room (ch, argument)) != NULL)
        return wch;

    number = number_argument (argument, arg);
    count = 0;
    for (wch = char_list; wch != NULL; wch = wch->next) {
        if (wch->in_room == NULL)
            continue;
        if (!char_can_see_anywhere (ch, wch))
            continue;
        if (!is_name (arg, wch->name))
            continue;
        if (++count == number)
            return wch;
    }
    return NULL;
}

/* Find an obj in a list. */
OBJ_DATA *find_obj_list (CHAR_DATA *ch, OBJ_DATA *list, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
    for (obj = list; obj != NULL; obj = obj->next_content) {
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            return obj;
    }
    return NULL;
}

/* Find an obj in player's inventory. */
OBJ_DATA *find_obj_char (CHAR_DATA *ch, CHAR_DATA *victim, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
    for (obj = victim->carrying; obj != NULL; obj = obj->next_content) {
        if (obj->wear_loc != WEAR_NONE)
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            return obj;
    }
    return NULL;
}

OBJ_DATA *find_carry (CHAR_DATA *ch, char *argument) {
    return find_obj_char (ch, ch, argument);
}

/* Find an obj in player's equipment. */
OBJ_DATA *find_eq (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
        if (obj->wear_loc == WEAR_NONE)
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            return obj;
    }
    return NULL;
}

/* Find an obj in the room or in inventory. */
OBJ_DATA *find_obj_here (CHAR_DATA * ch, char *argument) {
    OBJ_DATA *obj;
    if ((obj = find_obj_list (ch, ch->in_room->contents, argument)) != NULL)
        return obj;
    else if ((obj = find_carry (ch, argument)) != NULL)
        return obj;
    else if ((obj = find_eq (ch, argument)) != NULL)
        return obj;
    else
        return NULL;
}

/* Find an obj in the world. */
OBJ_DATA *find_obj_world (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ((obj = find_obj_here (ch, argument)) != NULL)
        return obj;

    number = number_argument (argument, arg);
    count = 0;
    for (obj = object_list; obj != NULL; obj = obj->next) {
        if (!char_can_see_obj (ch, obj))
            continue;
        if (!is_name (arg, obj->name))
            continue;
        if (++count == number)
            return obj;
    }
    return NULL;
}

/* get an object from a shopkeeper's list */
OBJ_DATA *find_obj_keeper (CHAR_DATA * ch, CHAR_DATA * keeper,
    char *argument) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument (argument, arg);
    count = 0;
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
            return obj;

        /* skip other objects of the same name */
        while (obj->next_content != NULL
               && obj->pIndexData == obj->next_content->pIndexData
               && !str_cmp (obj->short_descr, obj->next_content->short_descr))
            obj = obj->next_content;
    }
    return NULL;
}
