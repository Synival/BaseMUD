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

#include "resets.h"

#include "chars.h"
#include "items.h"
#include "mobiles.h"
#include "objs.h"
#include "recycle.h"
#include "rooms.h"
#include "utils.h"

/* Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c */
void reset_to_area (RESET_T *reset, AREA_T *area) {
    LIST2_REASSIGN_BACK (reset, area, area_prev, area_next,
        area, reset_first, reset_last);
}

void reset_to_room (RESET_T *reset, ROOM_INDEX_T *room) {
    reset_to_area (reset, (room == NULL) ? NULL : room->area);
    LIST2_REASSIGN_BACK (reset, room, room_prev, room_next,
        room, reset_first, reset_last);
}

/* (originally redit_add_reset() from OLC) */
void reset_to_room_before (RESET_T *reset, ROOM_INDEX_T *room, int index) {
    RESET_T *after;

    /* Support assigning to nothing. */
    if (room == NULL)
        return reset_to_room (reset, NULL);

    /* Assigning to a negative index assigns to the back, which is the
     * default behavior. */
    if (index < 0)
        return reset_to_room (reset, room);

    /* Assign to the area and the room, but don't insert into the list
     * just yet. */
    reset_to_area (reset, room->area);
    if (reset->room)
        LIST2_REMOVE (reset, room_prev, room_next,
            reset->room->reset_first, reset->room->reset_last);
    reset->room = room;

    /* No resets or first slot (0) selected. */
    if (!room->reset_first || index == 0) {
        LIST2_FRONT (reset, room_prev, room_next,
            room->reset_first, room->reset_last);
    }
    else {
        int reset_n = 1;
        for (after = room->reset_first; after->room_next; after = after->room_next)
            if (reset_n++ == index)
                break;
        LIST2_INSERT_AFTER (reset, after, room_prev, room_next,
            room->reset_first, room->reset_last);
    }
}

static CHAR_T *reset_last_mob = NULL;
static OBJ_T  *reset_last_obj = NULL;
static bool    reset_last_created = FALSE;

void reset_run_all (ROOM_INDEX_T *room) {
    RESET_T *reset;

    reset_last_mob = NULL;
    reset_last_obj = NULL;
    reset_last_created = FALSE;

    for (reset = room->reset_first; reset; reset = reset->room_next)
        reset_run (reset);

    reset_last_mob = NULL;
    reset_last_obj = NULL;
    reset_last_created = FALSE;
}

void reset_run (RESET_T *reset) {
    MOB_INDEX_T *mob_index;
    OBJ_INDEX_T *obj_index;
    OBJ_INDEX_T *obj_to_index;
    ROOM_INDEX_T *room;
    ROOM_INDEX_T *room_index;
    ROOM_INDEX_T *room_index_prev;
    CHAR_T *mob;
    OBJ_T *obj;
    int level = 0;
    int count, limit;

    room = reset->room;
    switch (reset->command) {
        case 'M':
            BAIL_IF_BUG (!(mob_index = mobile_get_index (reset->v.mob.mob_vnum)),
                "reset_room_reset: 'M': bad mob_vnum %d.", reset->v.mob.mob_vnum);
            BAIL_IF_BUG (!(room_index = room_get_index (reset->v.mob.room_vnum)),
                "reset_room_reset: 'M': bad room_vnum %d.", reset->v.mob.room_vnum);

            if (mob_index->mob_count >= reset->v.mob.global_limit) {
                reset_last_created = FALSE;
                return;
            }

            count = 0;
            for (mob = room_index->people_first; mob != NULL;
                 mob = mob->room_next)
            {
                if (mob->mob_index != mob_index)
                    continue;
                if (++count >= reset->v.mob.room_limit)
                    break;
            }
            if (count >= reset->v.mob.room_limit) {
                reset_last_created = FALSE;
                return;
            }

            mob = mobile_create (mob_index);

            /* Some more hard coding. */
            if (room_is_dark (room_index))
                SET_BIT (mob->affected_by, AFF_INFRARED);

            /* Pet shop mobiles get ACT_PET set. */
            room_index_prev = room_get_index (room_index->vnum - 1);
            if (room_index_prev && IS_SET (room_index_prev->room_flags, ROOM_PET_SHOP))
                EXT_SET (mob->ext_mob, MOB_PET);

            char_to_room (mob, room_index);
            reset_last_mob = mob;
            level = URANGE (0, mob->level - 2, LEVEL_HERO - 1); /* -1 ROM */
            reset_last_created = TRUE;
            break;

        case 'O': {
            int obj_count = UMAX(1, reset->v.obj.room_limit);

            if (!(obj_index = obj_get_index (reset->v.obj.obj_vnum))) {
                bug ("reset_room_reset: 'O': bad obj_vnum %d.", reset->v.obj.obj_vnum);
                bugf ("%d %d %d %d",
                    reset->v.value[1], reset->v.value[2],
                    reset->v.value[3], reset->v.value[4]);
                return;
            }
            if (!(room_index = room_get_index (reset->v.obj.room_vnum))) {
                bug ("reset_room_reset: 'O': bad room_vnum %d.", reset->v.obj.room_vnum);
                bugf ("%d %d %d %d",
                    reset->v.value[1], reset->v.value[2],
                    reset->v.value[3], reset->v.value[4]);
                return;
            }

            if (room_index->area->nplayer > 0 ||
                obj_index_count_in_list (obj_index, room_index->content_first)
                    >= obj_count)
            {
                reset_last_created = FALSE;
                return;
            }

            obj = obj_create (obj_index,    /* UMIN - ROM OLC */
               UMIN (number_fuzzy (level), LEVEL_HERO - 1));
            if (!item_has_worth_as_room_reset (obj))
                obj->cost = 0;
            obj_give_to_room (obj, room_index);
            reset_last_created = TRUE;
            break;
        }

        case 'P':
            BAIL_IF_BUG (!(obj_index = obj_get_index (reset->v.put.obj_vnum)),
                "reset_room_reset: 'P': bad obj_vnum %d.", reset->v.put.obj_vnum);
            BAIL_IF_BUG (!(obj_to_index = obj_get_index (reset->v.put.into_vnum)),
                "reset_room_reset: 'P': bad into_vnum %d.", reset->v.put.into_vnum);
            BAIL_IF_BUG (!item_index_is_container (obj_to_index),
                "reset_room_reset: 'P': to object %d is not a container.",
                    reset->v.put.into_vnum);

            if (reset->v.put.global_limit > 50) /* old format */
                limit = 6;
            else if (reset->v.put.global_limit == -1) /* no limit */
                limit = 999;
            else
                limit = reset->v.put.global_limit;

            if (room->area->nplayer > 0                                    ||
                (reset_last_obj = obj_get_last_by_index (obj_to_index)) == NULL ||
                (reset_last_obj->in_room == NULL && !reset_last_created)   ||
                (obj_index->obj_count >= limit)                            ||
                (count = obj_index_count_in_list (obj_index,
                    reset_last_obj->content_first)) > reset->v.put.put_count)
            {
                reset_last_created = FALSE;
                return;
            }

            while (count < reset->v.put.put_count) {
                obj = obj_create (obj_index,
                    number_fuzzy (reset_last_obj->level));
                obj_give_to_obj (obj, reset_last_obj);
                if (++obj_index->obj_count >= limit)
                    break;
            }

            /* fix object lock state! */
            reset_last_obj->v.container.flags =
                reset_last_obj->obj_index->v.container.flags;
            reset_last_created = TRUE;
            break;

        case 'G':
        case 'E': {
            struct reset_values_give  *gv = &(reset->v.give);
            struct reset_values_equip *eq = &(reset->v.equip);
            char cmd = reset->command;
            int obj_vnum, global_limit;

            obj_vnum     = (cmd == 'G') ? gv->obj_vnum     : eq->obj_vnum;
            global_limit = (cmd == 'G') ? gv->global_limit : eq->global_limit;

            BAIL_IF_BUG (!(obj_index = obj_get_index (obj_vnum)),
                "reset_room_reset: 'E' or 'G': bad obj_vnum %d.", obj_vnum);

            /* If we haven't instantiated the previous mob (at least we
             * ASSUME the command was 'M'), don't give or equip. */
            if (!reset_last_created)
                return;

            if (!reset_last_mob) {
                reset_last_created = FALSE;
                bug ("reset_room_reset: 'E' or 'G': null mob for vnum %d.",
                    obj_vnum);
                return;
            }

#ifdef BASEMUD_LOG_EQUIP_WARNINGS
            /* Show warnings for items in bad slots. */
            if (cmd == 'E') {
                flag_t wear_flag = wear_loc_get_flag (eq->wear_loc);
                if (!obj_index_can_wear_flag (obj_index, wear_flag)) {
                    bugf ("Warning: 'E' for object %d (%s) into unequippable "
                          "location '%s'.",
                        obj_vnum, obj_index->short_descr,
                        wear_loc_get_name (eq->wear_loc));
                }
                if (!char_has_available_wear_loc (reset_last_mob, eq->wear_loc)) {
                    bugf ("Warning: 'E' for object %d (%s) on mob %d (%s), "
                          "wear loc '%s' is already taken.",
                        obj_vnum, obj_index->short_descr,
                        reset_last_mob->mob_index->vnum, reset_last_mob->short_descr,
                        wear_loc_get_name (eq->wear_loc));
                }
            }
#endif

            /* Shop-keeper? */
            if (reset_last_mob->mob_index->shop) {
                int olevel = (obj_index->new_format)
                    ? 0 : item_index_get_old_reset_shop_level (obj_index);
                obj = obj_create (obj_index, olevel);
                SET_BIT (obj->extra_flags, ITEM_INVENTORY); /* ROM OLC */

#if 0 /* envy version */
                if (reset->command == 'G')
                    SET_BIT (obj->extra_flags, ITEM_INVENTORY);
#endif /* envy version */
            }
            /* ROM OLC else version */
            else {
                int limit;
                if (global_limit > 50) /* old format */
                    limit = 6;
                else if (global_limit == -1 || global_limit == 0) /* no limit */
                    limit = 999;
                else
                    limit = global_limit;

                /* Allow going over the global item limit 25% of the time. */
                if (obj_index->obj_count >= limit && number_range (0, 4) != 0)
                    break;

                obj = obj_create (obj_index,
                    UMIN (number_fuzzy (level), LEVEL_HERO - 1));

                /* Warnings for various bad ideas. */
#ifdef BASEMUD_LOG_EQUIP_WARNINGS
                {
                const char *warn_type;
                if (obj->level > reset_last_mob->level + 3)
                    warn_type = "too strong";
                else if (
                    item_is_weapon (obj)  &&
                    reset->command == 'E' &&
                    obj->level < reset_last_mob->level - 5 &&
                    obj->level < 45)
                {
                    warn_type = "too weak";
                }
                else
                    warn_type = NULL;

                /* Was a warning found? */
                if (warn_type != NULL) {
                    log_f ("%4s - %s: ", (cmd == 'G') ? "Give" : "Equip",
                        warn_type);
                    log_f ("  Object: (VNUM %5d)(Level %2d) %s",
                        obj->obj_index->vnum,
                        obj->level,
                        obj->short_descr);
                    log_f ("     Mob: (VNUM %5d)(Level %2d) %s",
                        reset_last_mob->mob_index->vnum,
                        reset_last_mob->level,
                        reset_last_mob->short_descr);
                }
                }
#endif
            }

            obj_give_to_char (obj, reset_last_mob);
            if (cmd == 'E')
                char_equip_obj (reset_last_mob, obj, eq->wear_loc);
            reset_last_created = TRUE;
            break;
        }

        case 'D':
            break;

        case 'R': {
            EXIT_T *exit_obj;
            int d1, d2;

            BAIL_IF_BUG (!(room_index = room_get_index (reset->v.randomize.room_vnum)),
                "reset_room_reset: 'R': bad vnum %d.", reset->v.randomize.room_vnum);

            for (d1 = 0; d1 < reset->v.randomize.dir_count - 1; d1++) {
                d2 = number_range (d1, reset->v.randomize.dir_count - 1);
                exit_obj = room_index->exit[d1];
                room_index->exit[d1] = room_index->exit[d2];
                room_index->exit[d2] = exit_obj;
            }
            break;
        }

        default:
            bug ("reset_run: bad command %c.", reset->command);
            break;
    }
}

void reset_commit_all (void) {
    RESET_T *reset, *next;
    for (reset = reset_data_get_first(); reset != NULL; reset = next) {
        next = reset_data_get_next (reset);
        reset_commit (reset);
    }
}

void reset_commit (RESET_T *reset) {
    RESET_VALUE_T *v;
    EXIT_T *pexit;
    ROOM_INDEX_T *room_index;
    bool fail;

    v = &(reset->v);
    fail = FALSE;

    switch (reset->command) {
        case 'D':
            room_index = room_get_index (v->door.room_vnum);
            if (v->door.dir < 0 ||
                v->door.dir >= DIR_MAX ||
                !room_index ||
                !(pexit = room_index->exit[v->door.dir]) ||
                !IS_SET (pexit->rs_flags, EX_ISDOOR))
            {
                bugf ("fix_resets: 'D': exit %d, room %d not door.",
                      v->door.dir, v->door.room_vnum);
                fail = TRUE;
                break;
            }

            switch (v->door.locks) {
                case RESET_DOOR_NONE:
                    break;
                case RESET_DOOR_CLOSED:
                    SET_BIT (pexit->exit_flags, EX_CLOSED);
                    break;
                case RESET_DOOR_LOCKED:
                    SET_BIT (pexit->exit_flags, EX_CLOSED | EX_LOCKED);
                    break;
                default:
                    bug ("load_resets: 'D': bad 'locks': %d.",
                         v->door.locks);
                    break;
            }
            pexit->rs_flags = pexit->exit_flags;
            break;
    }

    /* Remove broken resets. */
    if (fail == TRUE) {
        reset_data_free (reset);
        return;
    }

    /* Door resets are removed - all others belong to their room. */
    if (reset->command == 'D') {
        reset_data_free (reset);
        return;
    }

    /* Complain if resets don't belong to any particular room. */
    if ((room_index = room_get_index (reset->room_vnum)) == NULL) {
        bugf ("fix_resets: '%c': room %d does not exist.",
            reset->command, reset->room_vnum);
        reset_data_free (reset);
        return;
    }

    /* Assign the reset to the room. */
    reset_to_room (reset, room_index);
}
