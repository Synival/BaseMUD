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

#include "globals.h"
#include "comm.h"
#include "rooms.h"
#include "utils.h"
#include "recycle.h"
#include "resets.h"

#include "areas.h"

void area_update_all (void) {
    AREA_T *area;
    for (area = area_first; area != NULL; area = area->global_next)
        area_update (area);
}

void area_update (AREA_T *area) {
    if (++area->age < 3)
        return;

    /* Check age and reset.
     * Note: Mud School resets every 3 minutes (not 15). */
    if ((!area->empty && (area->nplayer == 0 || area->age >= 15))
        || area->age >= 31)
    {
        ROOM_INDEX_T *room_index;

        area_reset (area);
        wiznetf (NULL, NULL, WIZ_RESETS, 0, 0,
            "%s has just been reset.", area->title);

        area->age = number_range (0, 3);
        room_index = room_get_index (ROOM_VNUM_SCHOOL);
        if (room_index != NULL && area == room_index->area)
            area->age = 15 - 2;
        else if (area->nplayer == 0)
            area->empty = TRUE;
    }
}

/* OLC
 * Reset one area. */
void area_reset (AREA_T *area) {
    ROOM_INDEX_T *room;
    int vnum;
    for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++)
        if ((room = room_get_index (vnum)))
            room_reset (room);
}

void area_reinsert_resets_in_room_order_all (void) {
    AREA_T *area;
    for (area = area_get_first(); area; area = area_get_next (area))
        area_reinsert_resets_in_room_order (area);
}

void area_reinsert_resets_in_room_order (AREA_T *area) {
    ROOM_INDEX_T *room;
    RESET_T *reset;

    for (room = area->room_first; room; room = room->area_next) {
        for (reset = room->reset_first; reset; reset = reset->room_next) {
            if (reset->area != room->area) {
                bugf ("area_reinsert_resets_in_room_order: Reset '%c' for room "
                      "'%s' (%d) does not share the same area (???)",
                    reset->command, room->name, room->vnum);
                continue;
            }

            /* Remove and put back in. */
            reset_to_area (reset, NULL);
            reset_to_area (reset, area);
        }
    }
}
