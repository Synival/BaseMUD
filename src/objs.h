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

#ifndef __ROM_OBJS_H
#define __ROM_OBJS_H

#include "merc.h"

/* Shorthand macros. */
#define CAN_WEAR_FLAG(obj, flag) (obj_can_wear_flag(obj, flag))
#define IS_OBJ_STAT(obj, stat)   (IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat) (IS_SET((obj)->v.weapon.flags, (stat)))

#define WEIGHT_MULT(obj)         (obj_get_weight_mult (obj))

/* Creation / destruction */
OBJ_DATA *obj_create (OBJ_INDEX_DATA * pObjIndex, int level);
OBJ_DATA *obj_create_money (int gold, int silver);
void obj_clone (OBJ_DATA * parent, OBJ_DATA * clone);
void obj_extract (OBJ_DATA *obj);

/* "Is" / "Can" functions. */
bool obj_is_container (OBJ_DATA *obj);
bool obj_can_fit_in (OBJ_DATA *obj, OBJ_DATA *container);
bool obj_is_furniture (OBJ_DATA * obj, flag_t bits);
bool obj_can_wear_flag (OBJ_DATA *obj, flag_t flag);
bool obj_index_can_wear_flag (OBJ_INDEX_DATA *obj, flag_t flag);

/* "Get" functions. */
int obj_count_users (OBJ_DATA *obj);
int obj_get_ac_type (OBJ_DATA *obj, int iWear, int type);
int obj_index_count_in_list (OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list);
int obj_get_carry_number (OBJ_DATA *obj);
int obj_get_weight (OBJ_DATA *obj);
int obj_get_true_weight (OBJ_DATA *obj);
int obj_furn_preposition_type (OBJ_DATA * obj, int position);
const char *obj_furn_preposition_base (OBJ_DATA * obj, int position,
    const char *at, const char *on, const char *in, const char *by);
const char *obj_furn_preposition (OBJ_DATA * obj, int position);
int obj_get_weight_mult (OBJ_DATA *obj);

/* Action functions. */
void obj_to_char (OBJ_DATA *obj, CHAR_DATA *ch);
void obj_from_char (OBJ_DATA *obj);
void obj_from_room (OBJ_DATA *obj);
void obj_to_room (OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex);
void obj_to_obj (OBJ_DATA *obj, OBJ_DATA *obj_to);
void obj_from_obj (OBJ_DATA *obj);
char *obj_format_to_char (OBJ_DATA *obj, CHAR_DATA *ch, bool fShort);
void obj_list_show_to_char (OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
    bool fShowNothing);
void obj_to_keeper (OBJ_DATA *obj, CHAR_DATA *ch);
void obj_enchant (OBJ_DATA *obj);
bool obj_set_exit_flag (OBJ_DATA *obj, flag_t exit_flag);
bool obj_remove_exit_flag (OBJ_DATA *obj, flag_t exit_flag);

/* Look-up functions. */
OBJ_DATA *obj_get_by_index (OBJ_INDEX_DATA *pObjIndex);

/* Misc. utility functions. */
flag_t obj_exit_flag_for_container (flag_t exit_flag);

#endif
