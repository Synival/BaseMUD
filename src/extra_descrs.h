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

#ifndef __ROM_EXTRA_DESCRS_H
#define __ROM_EXTRA_DESCRS_H

#include "merc.h"

/* Link management */
void extra_descr_to_room_index_front (EXTRA_DESCR_T *ed, ROOM_INDEX_T *room);
void extra_descr_to_room_index_back (EXTRA_DESCR_T *ed, ROOM_INDEX_T *room);
void extra_descr_to_obj_front (EXTRA_DESCR_T *ed, OBJ_T *obj);
void extra_descr_to_obj_back (EXTRA_DESCR_T *ed, OBJ_T *obj);
void extra_descr_to_obj_index_back (EXTRA_DESCR_T *ed, OBJ_INDEX_T *obj_index);
void extra_descr_unlink (EXTRA_DESCR_T *ed);
char *extra_descr_get_description (EXTRA_DESCR_T *list, const char *name);

#endif
