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

#include "extra_descrs.h"

#include "utils.h"

#include <stdio.h>

void extra_descr_to_room_index_front (EXTRA_DESCR_T *ed, ROOM_INDEX_T *room) {
    ed->parent_type = EXTRA_DESCR_ROOM_INDEX;
    ed->parent = room;
    LIST2_FRONT (ed, on_prev, on_next,
        room->extra_descr_first, room->extra_descr_last);
}

void extra_descr_to_room_index_back (EXTRA_DESCR_T *ed, ROOM_INDEX_T *room) {
    ed->parent_type = EXTRA_DESCR_ROOM_INDEX;
    ed->parent = room;
    LIST2_BACK (ed, on_prev, on_next,
        room->extra_descr_first, room->extra_descr_last);
}

void extra_descr_to_obj_front (EXTRA_DESCR_T *ed, OBJ_T *obj) {
    ed->parent_type = EXTRA_DESCR_OBJ;
    ed->parent = obj;
    LIST2_FRONT (ed, on_prev, on_next,
        obj->extra_descr_first, obj->extra_descr_last);
}

void extra_descr_to_obj_back (EXTRA_DESCR_T *ed, OBJ_T *obj) {
    ed->parent_type = EXTRA_DESCR_OBJ;
    ed->parent = obj;
    LIST2_BACK (ed, on_prev, on_next,
        obj->extra_descr_first, obj->extra_descr_last);
}

void extra_descr_to_obj_index_back (EXTRA_DESCR_T *ed, OBJ_INDEX_T *obj_index) {
    ed->parent_type = EXTRA_DESCR_OBJ_INDEX;
    ed->parent = obj_index;
    LIST2_BACK (ed, on_prev, on_next,
        obj_index->extra_descr_first, obj_index->extra_descr_last);
}

void extra_descr_unlink (EXTRA_DESCR_T *ed) {
    switch (ed->parent_type) {
        case EXTRA_DESCR_ROOM_INDEX: {
            ROOM_INDEX_T *room = ed->parent;
            if (room != NULL) {
                ed->parent = NULL;
                LIST2_REMOVE (ed, on_prev, on_next,
                    room->extra_descr_first, room->extra_descr_last);
            }
            break;
        }

        case EXTRA_DESCR_OBJ: {
            OBJ_T *obj = ed->parent;
            if (obj != NULL) {
                ed->parent = NULL;
                LIST2_REMOVE (ed, on_prev, on_next,
                    obj->extra_descr_first, obj->extra_descr_last);
            }
            break;
        }

        case EXTRA_DESCR_OBJ_INDEX: {
            OBJ_INDEX_T *obj_index = ed->parent;
            if (obj_index != NULL) {
                ed->parent = NULL;
                LIST2_REMOVE (ed, on_prev, on_next,
                    obj_index->extra_descr_first, obj_index->extra_descr_last);
            }
            break;
        }

        default:
            ed->parent = NULL;
    }

    ed->parent_type = EXTRA_DESCR_NONE;
}

/* Get an extra description from a list. */
char *extra_descr_get_description (EXTRA_DESCR_T *list, const char *name) {
    for (; list != NULL; list = list->on_next)
        if (str_in_namelist ((char *) name, list->keyword))
            return list->description;
    return NULL;
}
