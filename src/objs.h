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

#ifndef __ROM_OBJS_H
#define __ROM_OBJS_H

#include "merc.h"

/* Shorthand macros. */
#define IS_OBJ_STAT(obj, stat)   (IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat) (IS_SET((obj)->v.weapon.flags, (stat)))

/* Creation / destruction */
OBJ_T *obj_create (OBJ_INDEX_T *obj_index, int level);
OBJ_T *obj_create_money (int gold, int silver);
void obj_clone (OBJ_T *parent, OBJ_T *clone);
void obj_extract (OBJ_T *obj);

/* "Is" / "Can" functions. */
bool obj_can_wear_flag (const OBJ_T *obj, flag_t wear_flag);
bool obj_index_can_wear_flag (const OBJ_INDEX_T *obj, flag_t wear_flag);

/* "Get" functions. */
int obj_count_users (const OBJ_T *obj);
int obj_get_ac_type (const OBJ_T *obj, int wear_loc, int type);
int obj_index_count_in_list (const OBJ_INDEX_T *obj_index, const OBJ_T *list);
int obj_get_carry_number (const OBJ_T *obj);
int obj_get_weight (const OBJ_T *obj);
int obj_get_true_weight (const OBJ_T *obj);
const char *obj_furn_preposition_base (const OBJ_T *obj, int position,
    const char *at, const char *on, const char *in, const char *by);
const char *obj_furn_preposition (const OBJ_T *obj, int position);
ROOM_INDEX_T *obj_get_room (const OBJ_T *obj);

/* Object transfer functions. */
void obj_give_to_char (OBJ_T *obj, CHAR_T *ch);
void obj_give_to_room (OBJ_T *obj, ROOM_INDEX_T *room_index);
void obj_give_to_obj (OBJ_T *obj, OBJ_T *obj_to);
void obj_give_to_keeper (OBJ_T *obj, CHAR_T *ch);
void obj_take (OBJ_T *obj);
void obj_take_from_char (OBJ_T *obj);
void obj_take_from_room (OBJ_T *obj);
void obj_take_from_obj (OBJ_T *obj);

/* Action functions. */
char *obj_format_to_char (const OBJ_T *obj, const CHAR_T *ch, bool is_short);
void obj_list_show_to_char (const OBJ_T *list, CHAR_T *ch, bool is_short,
    bool show_nothing);
void obj_enchant (OBJ_T *obj);
void obj_poof (OBJ_T *obj);

/* Look-up functions. */
OBJ_T *obj_get_by_index (const OBJ_INDEX_T *obj_index);

#endif
