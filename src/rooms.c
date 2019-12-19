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
#include "lookup.h"
#include "utils.h"
#include "objs.h"
#include "interp.h"

#include "rooms.h"

/* True if room is dark. */
bool room_is_dark (const ROOM_INDEX_T *room_index) {
    const SUN_T *sun;
    int sect;

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
    return is_name (ch->name, room->owner);
}

/* True if room is private. */
bool room_is_private (const ROOM_INDEX_T *room_index) {
    CHAR_T *rch;
    int count;

    if (room_index->owner != NULL && room_index->owner[0] != '\0')
        return TRUE;

    count = 0;
    for (rch = room_index->people; rch != NULL; rch = rch->next_in_room)
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

char *door_keyword_to_name (const char *keyword, char *out_buf, size_t size) {
    if (keyword == NULL || keyword[0] == '\0')
        snprintf (out_buf, size, "door");
    else
        one_argument (keyword, out_buf);
    return out_buf;
}

void room_add_money (ROOM_INDEX_T *room, int gold, int silver) {
    OBJ_T *obj, *obj_next;
    for (obj = room->contents; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        switch (obj->index_data->vnum) {
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

/* Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c */
void room_take_reset (ROOM_INDEX_T *room, RESET_T *reset) {
    if (!room || !reset)
        return;
    reset->area = room->area;
    LISTB_BACK (reset, next, room->reset_first, room->reset_last);
}
