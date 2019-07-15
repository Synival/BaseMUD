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

/* TODO: lots of code can be put into tables. */
/* TODO: review the function names for consistency. */
/* TODO: remove any redundant functions, like simple lookup functions. */
/* TODO: char_wear_obj() is pretty awful :( */

/* True if room is dark. */
bool room_is_dark (ROOM_INDEX_DATA * pRoomIndex) {
    int sect;

    if (pRoomIndex->light > 0)
        return FALSE;
    if (IS_SET (pRoomIndex->room_flags, ROOM_DARK))
        return TRUE;

    sect = pRoomIndex->sector_type;
    if (sect == SECT_INSIDE || sect == SECT_CITY)
        return FALSE;
    if (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        return TRUE;

    return FALSE;
}

bool room_is_owner (ROOM_INDEX_DATA *room, CHAR_DATA *ch) {
    if (room->owner == NULL || room->owner[0] == '\0')
        return FALSE;
    return is_name (ch->name, room->owner);
}

/* True if room is private. */
bool room_is_private (ROOM_INDEX_DATA * pRoomIndex) {
    CHAR_DATA *rch;
    int count;

    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
        return TRUE;

    count = 0;
    for (rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room)
        count++;
    if (IS_SET (pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2)
        return TRUE;
    if (IS_SET (pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
        return TRUE;
    if (IS_SET (pRoomIndex->room_flags, ROOM_IMP_ONLY))
        return TRUE;
    return FALSE;
}

char room_colour_char (ROOM_INDEX_DATA * room) {
#ifndef VANILLA
    int sect;
    const struct sector_type *sect_data;

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

EXIT_DATA *room_get_opposite_exit (ROOM_INDEX_DATA *from_room, int dir,
    ROOM_INDEX_DATA **out_room)
{
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit, *pexit_rev;;

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

void room_add_money (ROOM_INDEX_DATA *room, int gold, int silver) {
    OBJ_DATA *obj, *obj_next;
    for (obj = room->contents; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        switch (obj->pIndexData->vnum) {
            case OBJ_VNUM_SILVER_ONE:
                silver += 1;
                obj_extract (obj);
                break;

            case OBJ_VNUM_GOLD_ONE:
                gold += 1;
                obj_extract (obj);
                break;

            case OBJ_VNUM_SILVER_SOME:
                silver += obj->value[0];
                obj_extract (obj);
                break;

            case OBJ_VNUM_GOLD_SOME:
                gold += obj->value[1];
                obj_extract (obj);
                break;

            case OBJ_VNUM_COINS:
                silver += obj->value[0];
                gold += obj->value[1];
                obj_extract (obj);
                break;
        }
    }
    obj_to_room (obj_create_money (gold, silver), room);
}
