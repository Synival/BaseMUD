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

#include "rooms.h"

#include "chars.h"
#include "db.h"
#include "globals.h"
#include "interp.h"
#include "lookup.h"
#include "mobiles.h"
#include "objs.h"
#include "recycle.h"
#include "resets.h"
#include "tables.h"
#include "utils.h"

/* Translates mob virtual number to its room index struct.
 * Hash table lookup. */
ROOM_INDEX_T *room_get_index (int vnum) {
    ROOM_INDEX_T *room_index;

    for (room_index = room_index_hash[vnum % MAX_KEY_HASH];
         room_index != NULL; room_index = room_index->hash_next)
        if (room_index->vnum == vnum)
            return room_index;

    /* NOTE: This did cause the server not to boot, but that seems a bit
     *       extreme. So we just return NULL instead, and keep on booting.
     *       -- Synival */
    if (in_boot_db) {
        bug ("room_get_index: bad vnum %d.", vnum);
     // exit (1);
    }

    return NULL;
}

/* random room generation procedure */
ROOM_INDEX_T *room_get_random_index (CHAR_T *ch) {
    ROOM_INDEX_T *room;

    while (1) {
        room = room_get_index (number_range (0, 65535));
        if (room == NULL)
            continue;
        if (!char_can_see_room (ch, room))
            continue;
        if (room_is_private (room))
            continue;
        if (IS_SET (room->room_flags, ROOM_PRIVATE))
            continue;
        if (IS_SET (room->room_flags, ROOM_SOLITARY))
            continue;
        if (IS_SET (room->room_flags, ROOM_SAFE))
            continue;
        if (IS_NPC (ch) || !IS_SET (room->room_flags, ROOM_LAW))
            break;
    }

    return room;
}

/* True if room is dark. */
bool room_is_dark (const ROOM_INDEX_T *room_index) {
    const SUN_T *sun;
    int sect;

    RETURN_IF_BUG (room_index == NULL,
        "room_is_dark(): room is NULL.", 0, FALSE);

    if (room_index->light > 0)
        return FALSE;
    if (IS_SET (room_index->room_flags, ROOM_DARK))
        return TRUE;

    sect = room_index->sector_type;
    sun = sun_get_current();
    if (sect == SECT_INSIDE || sect == SECT_CITY)
        return FALSE;
    if (sun->is_dark)
        return TRUE;

    return FALSE;
}

bool room_is_owner (const ROOM_INDEX_T *room, const CHAR_T *ch) {
    if (room->owner == NULL || room->owner[0] == '\0')
        return FALSE;
    return str_in_namelist (ch->name, room->owner);
}

/* True if room is private. */
bool room_is_private (const ROOM_INDEX_T *room_index) {
    CHAR_T *rch;
    int count;

    if (room_index->owner != NULL && room_index->owner[0] != '\0')
        return TRUE;

    count = 0;
    for (rch = room_index->people_first; rch != NULL; rch = rch->room_next)
        count++;
    if (IS_SET (room_index->room_flags, ROOM_PRIVATE) && count >= 2)
        return TRUE;
    if (IS_SET (room_index->room_flags, ROOM_SOLITARY) && count >= 1)
        return TRUE;
    if (IS_SET (room_index->room_flags, ROOM_IMP_ONLY))
        return TRUE;
    return FALSE;
}

char room_colour_char (const ROOM_INDEX_T *room) {
#ifdef BASEMUD_COLOR_ROOMS_BY_SECTOR
    int sect;
    const SECTOR_T *sect_data;

    if (room == NULL)
        return 's';

    sect = room->sector_type;
    sect_data = &(sector_table[UMIN (SECT_MAX, sect)]);

    if (sect_data == NULL)
        return 's';
    if (sect_data->colour_char == '\0')
        return 's';

    return sect_data->colour_char;
#else
    return 's';
#endif
}

EXIT_T *room_get_opposite_exit (const ROOM_INDEX_T *from_room, int dir,
    ROOM_INDEX_T **out_room)
{
    ROOM_INDEX_T *to_room;
    EXIT_T *pexit, *pexit_rev;;

    if (from_room == NULL)
        return NULL;
    if ((pexit = from_room->exit[dir]) == NULL)
        return NULL;
    if ((to_room = pexit->to_room) == NULL)
        return NULL;
    if ((pexit_rev = to_room->exit[REV_DIR(dir)]) == NULL)
        return NULL;
    if (pexit_rev->to_room != from_room)
        return NULL;
    if (out_room)
        *out_room = to_room;
    return pexit_rev;
}

char *room_get_door_name (const char *keyword, char *out_buf, size_t size) {
    if (keyword == NULL || keyword[0] == '\0')
        snprintf (out_buf, size, "door");
    else
        one_argument (keyword, out_buf);
    return out_buf;
}

void room_add_money (ROOM_INDEX_T *room, int gold, int silver) {
    OBJ_T *obj, *obj_next;
    for (obj = room->content_first; obj != NULL; obj = obj_next) {
        obj_next = obj->content_next;
        switch (obj->obj_index->vnum) {
            case OBJ_VNUM_SILVER_ONE:
                silver += 1;
                obj_extract (obj);
                break;

            case OBJ_VNUM_GOLD_ONE:
                gold += 1;
                obj_extract (obj);
                break;

            case OBJ_VNUM_SILVER_SOME:
                silver += obj->v.money.silver;
                obj_extract (obj);
                break;

            case OBJ_VNUM_GOLD_SOME:
                gold += obj->v.money.gold;
                obj_extract (obj);
                break;

            case OBJ_VNUM_COINS:
                silver += obj->v.money.silver;
                gold   += obj->v.money.gold;
                obj_extract (obj);
                break;
        }
    }
    obj_give_to_room (obj_create_money (gold, silver), room);
}

OBJ_T *room_get_obj_of_type (const ROOM_INDEX_T *room, const CHAR_T *ch,
    int type)
{
    OBJ_T *obj;
    for (obj = room->content_first; obj != NULL; obj = obj->content_next)
        if (obj->item_type == type && (!ch || char_can_see_obj (ch, obj)))
            return obj;
    return NULL;
}

OBJ_T *room_get_obj_with_condition (const ROOM_INDEX_T *room, const CHAR_T *ch,
    bool (*cond) (const OBJ_T *obj))
{
    OBJ_T *obj;
    for (obj = room->content_first; obj != NULL; obj = obj->content_next)
        if ((!ch || char_can_see_obj (ch, obj)) && cond (obj))
            return obj;
    return NULL;
}

/* OLC
 * Reset one room.  Called by reset_area and olc. */
void room_reset (ROOM_INDEX_T *room) {
    if (!room)
        return;
    room_reset_exits (room);
    reset_run_all (room);
}

void room_reset_exits (ROOM_INDEX_T *room) {
    EXIT_T *exit_obj, *exit_rev;
    int exit_n;

    for (exit_n = 0; exit_n < DIR_MAX; exit_n++) {
        if ((exit_obj = room->exit[exit_n]) == NULL)
            continue;
     /* if (IS_SET (exit_obj->exit_flags, EX_BASHED))
            continue; */
        exit_obj->exit_flags = exit_obj->rs_flags;

        if (exit_obj->to_room == NULL)
            continue;
        if ((exit_rev = exit_obj->to_room->exit[door_table[exit_n].reverse])
                == NULL)
            continue;
        exit_rev->exit_flags = exit_rev->rs_flags;
    }
}

void room_to_area (ROOM_INDEX_T *room, AREA_T *area) {
    LIST2_REASSIGN_BACK (
        room, area, area_prev, area_next,
        area, room_first, room_last);
}

void room_index_to_hash (ROOM_INDEX_T *room) {
    int hash = db_hash_from_vnum (room->vnum);
    LIST2_FRONT (room, hash_prev, hash_next,
        room_index_hash[hash], room_index_hash_back[hash]);
}

void room_index_from_hash (ROOM_INDEX_T *room) {
    int hash = db_hash_from_vnum (room->vnum);
    LIST2_REMOVE (room, hash_prev, hash_next,
        room_index_hash[hash], room_index_hash_back[hash]);
}

void room_link_exits_by_vnum_all (void) {
    ROOM_INDEX_T *r;
    for (r = room_index_get_first(); r != NULL; r = room_index_get_next (r))
        room_link_exits_by_vnum (r);
}

void room_link_exits_by_vnum (ROOM_INDEX_T *room_index) {
#ifdef BASEMUD_LOG_KEY_WARNINGS
    OBJ_INDEX_T *key;
#endif
    EXIT_T *pexit;
    bool exit_found;
    int door;
    bool old_boot_db;

    exit_found = FALSE;
    for (door = 0; door < DIR_MAX; door++) {
        if ((pexit = room_index->exit[door]) == NULL)
            continue;
        exit_found = TRUE;
        if (pexit->to_vnum <= 0 || room_get_index (pexit->to_vnum) == NULL)
            exit_to_room_index_to (pexit, NULL);
        else
            exit_to_room_index_to (pexit, room_get_index (pexit->to_vnum));

        old_boot_db = in_boot_db;
        in_boot_db = FALSE;

#ifdef BASEMUD_LOG_KEY_WARNINGS
        if (pexit->key >= KEY_VALID && !(key = obj_get_index (pexit->key)))
            bugf ("Warning: Cannot find key %d for room %d exit %d",
                pexit->key, room_index->vnum, door);
#endif
        in_boot_db = old_boot_db;
    }

    /* TODO: this is out of scope of this function - where should it go? */
    /* Rooms with no exits are not allowed to have mobs. */
    if (!exit_found)
        SET_BIT (room_index->room_flags, ROOM_NO_MOB);
}

void room_fix_two_way_exits_all (void) {
    ROOM_INDEX_T *r;
    for (r = room_index_get_first(); r != NULL; r = room_index_get_next (r))
        room_fix_two_way_exits (r);
}

void room_fix_two_way_exits (ROOM_INDEX_T *room) {
    ROOM_INDEX_T *to_room;
    EXIT_T *pexit, *pexit_rev;
    int door, rev_dir;

    for (door = 0; door < DIR_MAX; door++) {
        if ((pexit = room->exit[door]) == NULL)
            continue;
        if ((to_room = pexit->to_room) == NULL)
            continue;
        rev_dir = REV_DIR(door);
        if ((pexit_rev = to_room->exit[rev_dir]) == NULL)
            continue;

        room_fix_two_way_exit_doors (room, door, to_room, rev_dir);

        /* If the reverse exit does not lead to the same location,
         * print a warning. Make an exception for the 'immort.are' zone. */
#ifdef BASEMUD_LOG_EXIT_WARNINGS
        if (pexit_rev->to_room != room &&
            !(room->vnum >= 1200 && room->vnum <= 1299))
        {
            bugf ("Warning: Exit %d[%s] reverse exit %d[%s] "
                  "leads to wrong room (%d).",
                room->vnum, door_get_name(door),
                to_room->vnum, door_get_name(rev_dir),
                (pexit_rev->to_room == NULL)
                     ? 0 : pexit_rev->to_room->vnum
            );
        }
#endif
    }
}

void room_fix_two_way_exit_doors (ROOM_INDEX_T *room_from, int dir_from,
                                  ROOM_INDEX_T *room_to,   int dir_to)
{
    EXIT_T *exit_from = room_from->exit[dir_from];
    EXIT_T *exit_to   = room_to  ->exit[dir_to];
    flag_t flags_from = exit_from->exit_flags;
    flag_t flags_to   = exit_to  ->exit_flags;

    if (dir_to != REV_DIR(dir_from))
        return;
    if (exit_from->to_room != room_to || exit_to->to_room != room_from)
        return;

    if (IS_SET(flags_from, EX_ISDOOR) && !IS_SET(flags_to, EX_ISDOOR)) {
#ifdef BASEMUD_LOG_EXIT_WARNINGS
        bugf ("Warning: Exit %d[%s, %s] is a door but %d[%s, %s] is not.",
            room_from->vnum, door_get_name (dir_from), exit_from->keyword,
            room_to  ->vnum, door_get_name (dir_to),   exit_to  ->keyword);
#endif
        return;
    }

    /* We only care about doors from now on. */
    if (!(IS_SET(flags_from, EX_ISDOOR) && IS_SET(flags_to, EX_ISDOOR)))
        return;

    /* Doors closed or locked on one side should definitely be
     * on the other. Make sure this is the case. */
    if (IS_SET (flags_from, EX_CLOSED) || IS_SET (flags_to, EX_CLOSED)) {
        SET_BIT (exit_from->exit_flags, EX_CLOSED);
        SET_BIT (exit_from->rs_flags,   EX_CLOSED);
        SET_BIT (exit_to->exit_flags,   EX_CLOSED);
        SET_BIT (exit_to->rs_flags,     EX_CLOSED);
    }
    if (IS_SET (flags_from, EX_LOCKED) || IS_SET (flags_to, EX_LOCKED)) {
        SET_BIT (exit_from->exit_flags, EX_LOCKED);
        SET_BIT (exit_from->rs_flags,   EX_LOCKED);
        SET_BIT (exit_to->exit_flags,   EX_LOCKED);
        SET_BIT (exit_to->rs_flags,     EX_LOCKED);
    }

    /* Different keys is bad! But we don't care if one side is pick-proof
     * and the other isn't. */
    if (exit_from->key != exit_to->key &&
        exit_from->key != KEY_NOKEYHOLE &&
        exit_to->key   != KEY_NOKEYHOLE)
    {
#ifdef BASEMUD_LOG_KEY_WARNINGS
        bugf ("Warning: Exits %d[%s] and %d[%s] have "
            "different keys [%d, %d]",
            room_from->vnum, door_get_name (dir_from),
            room_to  ->vnum, door_get_name (dir_to),
            exit_from->key, exit_to->key);
#endif
    }
}

int room_check_resets_all (void) {
    ROOM_INDEX_T *r;
    int error_count = 0;

    for (r = room_index_get_first(); r != NULL; r = room_index_get_next (r))
        error_count += room_check_resets (r);

    return error_count;
}

int room_check_resets (ROOM_INDEX_T *room) {
    RESET_T *reset;
    RESET_VALUE_T *v;
    ROOM_INDEX_T *last_room, *last_obj_room;
    int error_count;

    last_room     = NULL;
    last_obj_room = NULL;
    error_count   = 0;

    for (reset = room->reset_first; reset; reset = reset->room_next) {
        v = &(reset->v);
        switch (reset->command) {
            case 'M':
                mobile_get_index (v->mob.mob_vnum);
                last_room = room_get_index (v->mob.room_vnum);
                break;

            case 'O':
                obj_get_index (v->obj.obj_vnum);
                last_obj_room = room_get_index (v->obj.room_vnum);
                break;

            case 'P':
                obj_get_index (v->obj.obj_vnum);
                if (last_obj_room == NULL) {
                    bug ("fix_exits: reset 'P' in room %d with last_obj_room NULL",
                        room->vnum);
                    error_count++;
                }
                break;

            case 'G':
                obj_get_index (v->give.obj_vnum);
                if (last_room == NULL) {
                    bug ("fix_exits: reset 'G' in room %d with last_room NULL",
                        room->vnum);
                    error_count++;
                }
                last_obj_room = last_room;
                break;

            case 'E':
                obj_get_index (v->equip.obj_vnum);
                if (last_room == NULL) {
                    bug ("fix_exits: reset 'E' in room %d with last_room NULL",
                        room->vnum);
                    error_count++;
                }
                last_obj_room = last_room;
                break;

            case 'D':
                bug ("fix_exits: door reset still attached to room %d",
                    room->vnum);
                error_count++;
                break;

            case 'R': {
                room_get_index (v->randomize.room_vnum);
                int dir_count = v->randomize.dir_count;
                if (dir_count < 0) {
                    bugf ("fix_exits: reset 'R' in room %d with dir_count %d < 0",
                        room->vnum, dir_count);
                    error_count++;
                }
                else if (dir_count > DIR_MAX) {
                    bugf ("fix_exits: reset 'R' in room %d with dir_count %d > DIR_MAX",
                        room->vnum, dir_count);
                    error_count++;
                }
                break;
            }

            default:
                bugf ("fix_exits: room %d with reset cmd %c",
                    room->vnum, reset->command);
                error_count++;
                break;
        }
    }

    return error_count;
}

EXIT_T *room_create_exit (ROOM_INDEX_T *room_index, int dir) {
    EXIT_T *new;
    new = exit_new ();
    new->orig_door = dir;
    exit_to_room_index_from (new, room_index, dir);
    return new;
}

void exit_to_room_index_from (EXIT_T *exit, ROOM_INDEX_T *room, int dir) {
    if (exit->from_room == room)
        return;
    BAIL_IF_BUGF (room != NULL && room->exit[dir] != NULL,
        "exit_to_room_index: Room '%s' already has exit '%d'",
        room->name, dir);

    if (exit->from_room) {
        int dir;
        for (dir = 0; dir < DIR_MAX; dir++)
            if (exit->from_room->exit[dir] == exit)
                exit->from_room->exit[dir] = NULL;
        exit->from_room = NULL;
    }

    exit->from_room = room;
    if (room != NULL)
        room->exit[dir] = exit;
}

EXIT_T *room_get_orig_exit (const ROOM_INDEX_T *room, int dir) {
    int i;
    for (i = 0; i < DIR_MAX; i++)
        if (room->exit[i] && room->exit[i]->orig_door == dir)
            return room->exit[i];
    return NULL;
}

void exit_to_room_index_to (EXIT_T *exit, ROOM_INDEX_T *room) {
    if (exit->to_room == room)
        return;

    exit->to_room = room;
    if (room != NULL) {
        exit->to_anum = room->anum;
        exit->to_vnum = room->vnum;
        exit->to_area_vnum = room->area->vnum;
    }
    else {
        exit->to_anum = -1;
        exit->to_vnum = -1;
        exit->to_area_vnum = -1;
    }
}
